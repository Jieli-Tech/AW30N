#pragma bss_seg(".app_test.data.bss")
#pragma data_seg(".app_test.data")
#pragma const_seg(".app_test.text.const")
#pragma code_seg(".app_test.text")
#pragma str_literal_override(".app_test.text.const")

#include "audio_rf_mge.h"
#include "audio2rf_send.h"
#include "trans_packet.h"
#include "rf2audio_recv.h"
#include "rc_app.h"
#include "app_modules.h"
#include "app.h"

#include "cpu.h"
#include "config.h"
#include "typedef.h"
#include "vfs.h"
#include "hot_msg.h"
#include "msg.h"
#include "crc16.h"
#include "circular_buf.h"

#include "decoder_api.h"
#include "decoder_mge.h"
#include "decoder_msg_tab.h"
#include "encoder_mge.h"
#include "sound_mge.h"
#include "audio.h"
#include "audio_dac_api.h"
#include "audio_adc_api.h"
#include "audio_dac.h"
#include "audio_adc.h"
#include "ir_encoder.h"
#include "gpio.h"
#include "clock.h"
#include "bt_ble.h"
#include "vble_complete.h"
#include "usb/device/hid.h"
#include "hot_msg.h"
#include "power_api.h"
#include "tick_timer_driver.h"
#include "update.h"

#if ENCODER_UMP3_EN
#include "ump3_encoder.h"
#endif

#if DECODER_UMP3_EN
#include "ump3_encoder.h"
#endif

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[rf_controller]"
#include "log.h"

#if (RF_REMOTECONTROL_MODE_EN & TRANS_DATA_HID_EN)

/* 外部变量/函数声明 */
extern void bt_init_api(void);
extern void ble_module_enable(u8 en);
extern u32 rf_enc_output(void *priv, u8 *data, u16 len);

#define SPEAKER_START   1
#define SPEAKER_STOP    0

volatile u8 speaker_status = SPEAKER_STOP;
static void set_speaker_status(u8 status)
{
    local_irq_disable();
    speaker_status = status;
    local_irq_enable();
}
static void get_speaker_status()
{
    local_irq_disable();
    if (SPEAKER_STOP == speaker_status) {
        log_info("speaker status stop!!!\n");
    } else {
        log_info("speaker status working!!!\n");
    }
    local_irq_enable();
}

static int remote_audio_send_api(u8 *data, u16 len)
{
    return vble_slave_send_api(ATT_SLV2MSTR_RF_RADIO_IDX, data, len);
}
static int rc_check_status_api(void)
{
    int ble_status;
    vble_ioctl(VBLE_IOCTL_GET_SLAVE_STATUS, (int)&ble_status);
    if (ble_status == BLE_ST_NOTIFY_IDICATE) {
        return 1;
    } else {
        return 0;
    }
}
static int rc_get_valid_len_api(void)
{
    return get_buffer_vaild_len(0);
}
const audio2rf_send_mge_ops remote_ops = {
    .send = remote_audio_send_api,
    .check_status = rc_check_status_api,
    .get_valid_len = rc_get_valid_len_api,
};

/* 红外发射 */
const u8 key_cmd_tab[][2] = { //msg转cmd码表,匹配海信电视
    {MSG_UP, 0x16},
    {MSG_DOWN, 0x17},
    {MSG_POWER, 0x0d},
    {MSG_LEFT, 0x19},
    {MSG_RIGHT, 0x18},
    {MSG_VOL_UP, 0x44},
    {MSG_VOL_DOWN, 0x43},
    {MSG_RETURN, 0x48},
    {MSG_MAINPAGE, 0x94},
    {MSG_CONFIRM, 0x15},
    {MSG_MENU, 0x14},
};

u8 msg_to_key_cmd(int msg)
{

    u8 key_value = msg;
    for (u8 i = 0; i < ARRAY_SIZE(key_cmd_tab); i++) {
        if (key_value == key_cmd_tab[i][0]) {
            /* log_info("match 0x%x\n", hid_key_list[i][1]); */
            return key_cmd_tab[i][1];
        }
    }
    log_error("key_value no match, 0x%x\n", key_value);
    return 0;
}

static void send_ir_key(int msg, u8 repeat_flag)
{
    u8 cmd = msg_to_key_cmd(msg);
    if (cmd) {
        ir_encode_tx(0, cmd, repeat_flag);
    }
}

static u8 rc_ctrl_adc_buff[1280] AT(.rec_data);
void rec_cbuf_init(void *cbuf_t)
{
    cbuf_init(cbuf_t, &rc_ctrl_adc_buff[0], sizeof(rc_ctrl_adc_buff));
}

void rf_controller_uninit()
{
    if (SPEAKER_START == speaker_status) {
        audio2rf_encoder_stop();
        audio2rf_send_stop_packet();
        set_speaker_status(SPEAKER_STOP);
        log_info("SPEAKER_STOP\n");
    }

#if BLE_EN
    ble_module_enable(0);
#endif
    audio2rf_send_register(NULL);
    audio_adc_off_api();
    key_table_sel(NULL);

}
// consumer key
/* #define CONSUMER_VOLUME_INC             0x0001 */
/* #define CONSUMER_VOLUME_DEC             0x0002 */
/* #define CONSUMER_PLAY_PAUSE             0x0004 */
/* #define CONSUMER_MUTE                   0x0008 */
/* #define CONSUMER_SCAN_PREV_TRACK        0x0010 */
/* #define CONSUMER_SCAN_NEXT_TRACK        0x0020 */
/* #define CONSUMER_SCAN_FRAME_FORWARD     0x0040 */
/* #define CONSUMER_SCAN_FRAME_BACK        0x0080 */
const u16 msg_2_hidtab[][2] = { //msg转标准hid键值
    {MSG_UP, USB_AUDIO_PREFILE},
    {MSG_DOWN, USB_AUDIO_NEXTFILE},
    {MSG_LEFT, USB_AUDIO_REWIND},
    {MSG_RIGHT, USB_AUDIO_FASTFORWARD},
    {MSG_VOL_UP, USB_AUDIO_VOLUP},
    {MSG_VOL_DOWN, USB_AUDIO_VOLDOWN},
    {MSG_CONFIRM, USB_AUDIO_PP},
    {MSG_MUTE, USB_AUDIO_MUTE},
};

#if TRANS_DATA_HID_EN
extern int ble_hid_data_send(u8 report_id, u8 *data, u16 len);
#endif

void rf_send_hid_key_api(int msg, u16 len)
{
    u16 key_msg = msg;
    u16 key_value = 0;
    for (u8 i = 0; i < ARRAY_SIZE(msg_2_hidtab); i++) {
        if (key_msg == msg_2_hidtab[i][0]) {
            /* log_info("match 0x%x\n", hid_key_list[i][1]); */
            key_value = msg_2_hidtab[i][1];

        }
    }
    if (0 == key_value) {
        log_error("hid key_msg no match, 0x%x\n", key_msg);
        goto __key_no_march;
    }

#if TRANS_DATA_HID_EN
    vble_slave_send_api(ATT_SLV2MSTR_HID_IDX, (u8 *)&key_value, len);
    /* ble_hid_data_send(1, (u8 *)&key_value, len); */
#endif
__key_no_march:
    return;

}
u32 softoff_count_down = SYS_TIMER_SOFTOFF_TIME;
void set_softoff_countdown(u32 count_down)
{
    softoff_count_down = count_down;
}
#define IDLE_SYS_CLK         96000000
#define SEND_AUDIO_SYS_CLK  192000000
u32 get_softoff_countdown()
{
    return softoff_count_down;
}
void rf_rc_app()
{
    int msg[2];
    u32 err;
    clk_set("sys", IDLE_SYS_CLK);
    delay_10ms(30);
    key_table_sel(rc_key_msg_filter);
    audio_rf_clr_buf();
    u32 last_jiffies = 0;
    u32 ble_status = 0;

    /* ir_encoder_init(IO_PORTB_03, 38000, 5000); */
    /* 蓝牙BLE初始化 */
#if BLE_EN
    /* bt_init_api(); */
    vble_slave_init_api();
    audio2rf_send_register(&remote_ops);
#endif

    log_info("rf_controller\n");

    while (1) {
        if (SPEAKER_STOP == speaker_status) {
            /* putchar('s'); */
            /* u32 sr = dac_sr_read(); */
            audio_off();
            sys_power_down(2000000);
            audio_init();
            /* dac_power_on(sr); */
        }

        vble_ioctl(VBLE_IOCTL_GET_SLAVE_STATUS, (int)&ble_status);
        if ((BLE_ST_NOTIFY_IDICATE != ble_status) && (SPEAKER_START == speaker_status)) {
            audio2rf_encoder_stop();
            audio_adc_off_api();
            clk_set("sys", IDLE_SYS_CLK);
            delay_10ms(30);
            set_speaker_status(SPEAKER_STOP);
            log_info("SPEAKER_STOP\n");
#if SYS_TIMER_SOFTOFF
            set_softoff_countdown(SYS_TIMER_SOFTOFF_TIME);
#endif
        }

#if SYS_TIMER_SOFTOFF
        COUNTDOWN_BY_JIFFIES(last_jiffies, softoff_count_down)
        /* log_info("count down %d\n", get_softoff_countdown()); */
        if ((!get_softoff_countdown()) && (SPEAKER_STOP == speaker_status)) {
            /* 关闭模块进入softoff */
            sys_softoff();
        }

#endif
        err = get_msg(2, &msg[0]);
        switch (msg[0]) {
        case MSG_MENU:
        case MSG_UP:
        case MSG_MUTE:
        case MSG_VOL_DOWN:
        case MSG_CONFIRM:
        case MSG_VOL_UP:
        case MSG_RETURN:
        case MSG_DOWN:
        case MSG_MAINPAGE:
        case MSG_POWER:
        case MSG_LEFT:
        case MSG_RIGHT:
            /* send_ir_key(msg[0], 0); */
            vble_ioctl(VBLE_IOCTL_GET_SLAVE_STATUS, (int)&ble_status);
            if (BLE_ST_NOTIFY_IDICATE != ble_status) {
                break;
            }
            rf_send_hid_key_api(msg[0], sizeof(u16));
            //send_hid_data
            break;
        //io_key
        case MSG_SPEAKER://audio_data
            vble_ioctl(VBLE_IOCTL_GET_SLAVE_STATUS, (int)&ble_status);
            if (BLE_ST_NOTIFY_IDICATE != ble_status) {
                break;
            }
            if (SPEAKER_STOP == speaker_status) {
                clk_set("sys", SEND_AUDIO_SYS_CLK);
                delay_10ms(30);
                log_info("SPEAKER_START\n");
                set_speaker_status(SPEAKER_START);
                audio_adc_init_api(16000, AUDIO_ADC_MIC, 0);
                audio2rf_encoder_io(opus_encode_api, enc_input, rf_enc_output, INDEX_OPUS);
                /* audio2rf_encoder_io(ump3_encode_api, enc_input, rf_enc_output, INDEX_UMP3); */
                //send_audio_data
            } else {
                //send_audio_data
                audio2rf_encoder_stop();
                audio2rf_send_stop_packet();
                audio_adc_off_api();
                clk_set("sys", IDLE_SYS_CLK);
                delay_10ms(30);
                set_speaker_status(SPEAKER_STOP);
                log_info("SPEAKER_STOP\n");
            }

            break;
        case MSG_BLE_APP_UPDATE_START:
        /* 手机APP升级 */
        case MSG_BLE_TESTBOX_UPDATE_START:
        /* 测试盒蓝牙升级 */
        case MSG_UART_TESTBOX_UPDATE_START:
            /* 测试盒串口升级 */
            app_update_handle(msg[0]);
            break;
        case MSG_CHANGE_WORK_MODE:
            goto __remotecontrol_exit;
        case MSG_500MS:
            /* app_power_scan(); */
            /* log_info("ble status 0x%x\n", ble_status); */
            wdt_clear();
        default:
            /*     ap_handle_hotkey(msg[0]); */
            break;

        }
    }
__remotecontrol_exit:
    rf_controller_uninit();
}

#endif


