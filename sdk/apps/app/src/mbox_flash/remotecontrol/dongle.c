#pragma bss_seg(".app_dongle.data.bss")
#pragma data_seg(".app_dongle.data")
#pragma const_seg(".app_dongle.text.const")
#pragma code_seg(".app_dongle.text")
#pragma str_literal_override(".app_dongle.text.const")

#include "audio2rf_send.h"
#include "rf_send_queue.h"
#include "rf2audio_recv.h"
#include "trans_unpacket.h"
#include "rc_app.h"
#include "app_modules.h"
#include "app.h"

#include "cpu.h"
#include "config.h"
#include "typedef.h"
#include "vfs.h"
#include "msg.h"
#include "custom_event.h"
#include "crc16.h"
#include "circular_buf.h"

#include "decoder_api.h"
#include "decoder_mge.h"
#include "encoder_mge.h"
#include "sound_mge.h"
#include "audio.h"
#include "audio_dac_api.h"
#include "audio_adc_api.h"
#include "audio_dac.h"
#include "audio_adc.h"
#include "usb_stack.h"
#include "usb_common_def.h"
#include "uac_audio.h"
#include "hid.h"
#include "custom_hid.h"
#include "auadc_2_usbmic.h"
#include "third_party/common/ble_user.h"
#include "clock.h"
#if ENCODER_UMP3_EN
#include "ump3_encoder.h"
#endif
#include "bt_ble.h"
#include "vble_complete.h"
#include "update.h"
#include "trans_unpacket.h"
#include "spple_app_config.h"

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[rf_controller]"
#include "log.h"

#if (RF_REMOTECONTROL_MODE_EN & TRANS_DATA_SPPLE_EN)

/* 外部变量/函数声明 */
extern bool get_usb_mic_status();
extern int rf_dongle_hid_callback(void *priv, u8 *rf_packet, u16 packet_len);
extern sound_out_obj *usb_mic_get_stream_info(u32 sr, u32 frame_len, u32 ch, void **ppsrc, void **pkick);
extern void usb_mic_stream_clear_stream_info(void **ppsrc);
extern void set_usb_mic_func(void *open, void *close);
static void dongle_receiver_audio(u32 index, RADIO_PACKET_TYPE type, u8 *data, u16 len);

/* #define HIDKEY_REPORT_ID               0x1 */

/* static const u8 sHIDReportDesc_hidkey[] = { */
/*     0x05, 0x0C,        // Usage Page (Consumer) */
/*     0x09, 0x01,        // Usage (Consumer Control) */
/*     0xA1, 0x01,        // Collection (Application) */
/*     0x85, HIDKEY_REPORT_ID,  //   Report ID (1) */
/*     0x09, 0xE9,        //   Usage (Volume Increment) */
/*     0x09, 0xEA,        //   Usage (Volume Decrement) */
/*     0x09, 0xCD,        //   Usage (Play/Pause) */
/*     0x09, 0xE2,        //   Usage (Mute) */
/*     0x09, 0xB6,        //   Usage (Scan Previous Track) */
/*     0x09, 0xB5,        //   Usage (Scan Next Track) */
/*     0x09, 0xB3,        //   Usage (Fast Forward) */
/*     0x09, 0xB4,        //   Usage (Rewind) */
/*     0x15, 0x00,        //   Logical Minimum (0) */
/*     0x25, 0x01,        //   Logical Maximum (1) */
/*     0x75, 0x01,        //   Report Size (1) */
/*     0x95, 0x10,        //   Report Count (16) */
/*     0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position) */
/*     0xC0,              // End Collection */
/*     // 35 bytes */
/* }; */
/* static u8 get_report_id() */
/* { */
/*     return HIDKEY_REPORT_ID; */
/* } */

typedef struct __dongle_mge_struct {
    rev_fsm_mge packet_recv;
    /* RF_RADIO_ENC_HEAD enc_head; */
    sound_stream_obj stream;
    /* dec_obj *packet_recv.dec_obj; */
    cbuffer_t dec_icbuf;
    cbuffer_t ack_cbuf;
    /* cbuffer_t cmd_cbuf; */
    queue_obj g_dongle_queue;
    /* volatile u32 dongle_event_status; */
} dongle_mge_struct;
static dongle_mge_struct dongle_mge;

static u32 rf_recv_cnt = -1;
static u32 dongle_dec_ibuff[1024 * 2 / 4];
static u32 dongle_ack_buff[32 / 4];
static u8 dongle_packet_cmd_buff[6 * 10];
static u8 dongle_packet[16] ALIGNED(2);
extern volatile u8 rf_radio_status;

static void bt_usb_mic_hid_init()
{
    u32 usb_class = HID_CLASS | MIC_CLASS;
    usb_device_mode(0, 0);
    hid_init();
    uac_init();
#if RCSP_BTMATE_EN
    custom_hid_init();
    usb_class |= CUSTOM_HID_CLASS;
#endif
    set_usb_mic_func(usb_mic_get_stream_info, usb_mic_stream_clear_stream_info);
    usb_device_mode(0, usb_class);
}

static int dg_audio_send_api(u8 *data, u16 len)
{
    return vble_master_send_api(ATT_MSTR2SLV_RF_RADUI_IDX, data, len);
}
static int dg_check_status_api(void)
{
    int ble_status;
    vble_ioctl(VBLE_IOCTL_GET_MASTER_STATUS, (int)&ble_status);
    if (ble_status == 0x50) {
        return 1;
    } else {
        return 0;
    }
}
static int dg_get_valid_len_api(void)
{
    return get_buffer_vaild_len(0);
}
const audio2rf_send_mge_ops dongle_ops = {
    .send = dg_audio_send_api,
    .check_status = dg_check_status_api,
    .get_valid_len = dg_get_valid_len_api,
};


u32 dongle_rf_loop(void)
{
    rev_fsm_mge *p_recv_ops = &dongle_mge.packet_recv;
    u32 len = packet_cmd_get(&p_recv_ops->cmd_cbuf, &dongle_packet[0], sizeof(dongle_packet));
    if (0 == len) {
        return 0;
    }
    log_info("get_cmd_msg! 0x%x:%d 0x%x:%d \n", &dongle_packet[1], dongle_packet[1], &dongle_packet[0], dongle_packet[0]);
    RF_RADIO_ENC_HEAD *p_enc_head = (RF_RADIO_ENC_HEAD *)&dongle_packet[2];
    switch (dongle_packet[1]) {
    case AUDIO2RF_START_PACKET:
        log_info("MSG_RECEIVER_START");
        if (0 == get_usb_mic_status()) {
            break;
        }
        u8 ack;
        if (RADIO_RECEIVING == rf_radio_status) {
            ack = 1;
            audio2rf_ack_cmd(&dongle_mge.g_dongle_queue, AUDIO2RF_START_PACKET, &ack, sizeof(ack));
            break;
        }
        //准备数据源，相当于开文件
        log_info("dec_start!!!\n");
        cbuf_init(&dongle_mge.dec_icbuf, &dongle_dec_ibuff[0], sizeof(dongle_dec_ibuff));
        memset(&dongle_mge.stream, 0, sizeof(sound_stream_obj));
        dongle_mge.stream.p_ibuf =  &dongle_mge.dec_icbuf;
        dongle_mge.packet_recv.dec_obj = rf2audio_decoder_start(p_enc_head,  &dongle_mge.stream, MIC_AUDIO_RATE);
        if (NULL != dongle_mge.packet_recv.dec_obj) {
            //ack cmd
            set_usb_mic_info(dongle_mge.packet_recv.dec_obj);
            set_radio_status(RADIO_RECEIVING);
            ack = 1;
            audio2rf_ack_cmd(&dongle_mge.g_dongle_queue, AUDIO2RF_START_PACKET, &ack, sizeof(ack));
        } else {
            ack = 0;
            audio2rf_ack_cmd(&dongle_mge.g_dongle_queue, AUDIO2RF_START_PACKET, &ack, sizeof(ack));
        }

        break;
    case AUDIO2RF_STOP_PACKET:
        log_info("************ RX PACK_STOP ************\n");
        log_info("MSG_RECEIVER_STOP");
        if (RADIO_IDLE != rf_radio_status) {
            rf2audio_decoder_stop(dongle_mge.packet_recv.dec_obj, clr_usb_mic_info);
            dongle_mge.packet_recv.dec_obj = NULL;
            set_radio_status(RADIO_IDLE);
        }

        break;
    }
    return 0;
}


void dongle_app()
{
#if CONFIG_IOT_ENABLE

    int msg[2];
    u32 err;
    clk_set("sys", 120000000);
    delay_10ms(10);
    audio_rf_clr_buf();
    /* 蓝牙BLE初始化 */
    /* usb_hid_set_repport_map(sHIDReportDesc_hidkey, sizeof(sHIDReportDesc_hidkey)); */
    bt_usb_mic_hid_init();

    vble_master_init_api();

    memset(&dongle_mge, 0, sizeof(dongle_mge));

    cbuf_init(&dongle_mge.packet_recv.cmd_cbuf, &dongle_packet_cmd_buff[0], sizeof(dongle_packet_cmd_buff));
    vble_master_recv_cb_register(ATT_SLV2MSTR_HID_IDX,      &dongle_mge.packet_recv, rf_dongle_hid_callback);
    vble_master_recv_cb_register(ATT_SLV2MSTR_RF_RADIO_IDX, &dongle_mge.packet_recv, unpack_data_deal);

    decoder_init();

    cbuf_init(&dongle_mge.ack_cbuf, &dongle_ack_buff[0], sizeof(dongle_ack_buff));

    dongle_mge.g_dongle_queue.p_send_ops = (void *)&dongle_ops;
    rf_queue_init((void *)&dongle_mge.g_dongle_queue, (void *)&dongle_ops, &dongle_mge.ack_cbuf);
    u32 ble_status = 0;
    u32 app_event = 0;
    log_info("rf_receiver\n");

    while (1) {
        err = get_msg(2, &msg[0]);
        if (MSG_NO_ERROR != err) {
            msg[0] = NO_MSG;
            log_info("get msg err 0x%x\n", err);
        }
        dongle_rf_loop();

#if RCSP_BTMATE_EN
        extern void app_update_start(int msg);
        app_update_start(msg[0]);
#endif

        /* vble_ioctl(VBLE_IOCTL_GET_MASTER_STATUS, (int)&ble_status); */
        ble_status = dg_check_status_api();
        if (((!ble_status) && (RADIO_RECEIVING == rf_radio_status))) {
            rf2audio_decoder_stop(dongle_mge.packet_recv.dec_obj, clr_usb_mic_info);
            dongle_mge.packet_recv.dec_obj = NULL;
            set_radio_status(RADIO_IDLE);
        }
        switch (msg[0]) {
        case MSG_500MS:
            wdt_clear();
            break;
        }
    }
#else
    extern void spple_app_main();
    spple_app_main();
#endif
}
int rf_dongle_hid_callback(void *priv, u8 *rf_packet, u16 packet_len)
{
    u32 hid_key = *rf_packet;
    u8 usb_packet[packet_len + 1];
    memset(&usb_packet, 0, sizeof(usb_packet));
    usb_packet[0] = usb_get_hidkey_report_id();//report id
    memcpy(&usb_packet[1], rf_packet, packet_len);
    /* log_info_hexdump((u8 *)usb_packet, sizeof(usb_packet)); */
    /* log_info("usb_packet[1] 0x%x, usb_packet[2] 0x%x\n", usb_packet[1], usb_packet[2]); */
    extern void usb_write_hid_key(u8 * data, u16 len);
    usb_write_hid_key(usb_packet, packet_len + 1); //描述符定键长 + report_id
    //抬起
    memset(&usb_packet[1], 0, (sizeof(usb_packet) - 1));
    usb_write_hid_key(usb_packet, packet_len + 1);
    return packet_len;
}


#endif
