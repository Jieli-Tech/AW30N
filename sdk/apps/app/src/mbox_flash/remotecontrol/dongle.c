#pragma bss_seg(".app_test.data.bss")
#pragma data_seg(".app_test.data")
#pragma const_seg(".app_test.text.const")
#pragma code_seg(".app_test.text")
#pragma str_literal_override(".app_test.text.const")

#include "audio2rf_send.h"
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

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[rf_controller]"
#include "log.h"

#if (RF_REMOTECONTROL_MODE_EN & TRANS_DATA_SPPLE_EN)

/* 外部变量/函数声明 */
extern bool get_usb_mic_status();
extern bool rf_recv_data_check(void *rf_packet, u16 packet_len);
extern int rf_dongle_hid_callback(u8 *rf_packet, u16 packet_len);
extern sound_out_obj *usb_mic_get_stream_info(u32 sr, u32 frame_len, u32 ch, void **ppsrc, void **pkick);
extern void usb_mic_stream_clear_stream_info(void **ppsrc);
extern void set_usb_mic_func(void *open, void *close);

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

extern volatile u8 rf_radio_status;
void dongle_app()
{
    int msg[2];
    u32 err;
    clk_set("sys", 120000000);
    delay_10ms(10);
    audio_rf_clr_buf();
    /* 蓝牙BLE初始化 */
    /* usb_hid_set_repport_map(sHIDReportDesc_hidkey, sizeof(sHIDReportDesc_hidkey)); */
    bt_usb_mic_hid_init();

    vble_master_init_api();

    vble_master_recv_cb_register(ATT_SLV2MSTR_HID_IDX, rf_dongle_hid_callback);
    vble_master_recv_cb_register(ATT_SLV2MSTR_RF_RADIO_IDX, rf_receiver_audio_callback);
    decoder_init();

    u32 ble_status = 0;
    log_info("rf_receiver\n");

    while (1) {
        err = get_msg(2, &msg[0]);

#if RCSP_BTMATE_EN
        extern void app_update_start(int msg);
        app_update_start(msg[0]);
#endif

        vble_ioctl(VBLE_IOCTL_GET_MASTER_STATUS, (int)&ble_status);
        if (((0x50 != ble_status) && (RADIO_RECEIVING == rf_radio_status)) || (RADIO_IDLE == rf_radio_status)) {
            rf2audio_decoder_stop(STREAM_TO_USB);
            if (RADIO_RECEIVING == rf_radio_status) {
                set_radio_status(RADIO_IDLE);
            }
        }
        switch (msg[0]) {
        case MSG_500MS:
            wdt_clear();
            break;

        case MSG_RECEIVER_STOP://audio_data
            rf2audio_decoder_stop(STREAM_TO_USB);
            if (RADIO_RECEIVING == rf_radio_status) {
                set_radio_status(RADIO_IDLE);
            }
            break;
        case MSG_POWER:
            log_info("POWER\n");
            break;
        }
    }
}
int rf_dongle_hid_callback(u8 *rf_packet, u16 packet_len)
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

static u32 rf_recv_cnt = -1;
static rev_fsm_mge dongle_recv_ops;
static int dongle_receiver_audio(rev_fsm_mge *ops, u8 *buff, u16 packet_len)
{
    u32 stream_index = 0;
    bool res = ar_trans_unpack(ops, buff, packet_len, &stream_index);
    if (false == res) {
        return 0;
    }
    /* JL_PORTA->DIR &= ~BIT(3); */
    /* JL_PORTA->OUT |=  BIT(3); */

    /* log_info("R cnt:%d type:%d len:%d", ops->packet_index, ops->type, sizeof(RF_RADIO_PACKET) + ops->length); */
    /* log_info_hexdump((u8 *)packet, sizeof(RF_RADIO_PACKET)); */
    /* log_info_hexdump((u8 *)((u32)packet + sizeof(RF_RADIO_PACKET)),  len); */
    u8 *pdata = &buff[stream_index];


    switch (ops->type) {
    case AUDIO2RF_START_PACKET:
        log_info("************ RX PACK_HEADER ************\n");
        RF_RADIO_ENC_HEAD *p_enc_head = (RF_RADIO_ENC_HEAD *)pdata;
        rf2audio_decoder_start(p_enc_head, STREAM_TO_USB, MIC_AUDIO_RATE);
        rf_recv_cnt = ops->packet_index;
        break;
    case AUDIO2RF_DATA_PACKET:
        if (0 == get_usb_mic_status()) {
            break;
        }
        rf2audio_receiver_write_data(pdata, ops->length);
        if (rf_recv_cnt == ops->packet_index - 1) {
            rf_recv_cnt = ops->packet_index;
        } else {
            /* log_info("************ Recv index wrong!!! %d / %d\n", rf_recv_cnt, ops->packet_index); */
        }
        break;
    case AUDIO2RF_STOP_PACKET:
        log_info("************ RX PACK_STOP ************\n");
        rf_recv_cnt = -1;
        set_radio_status(RADIO_IDLE);
        post_msg(1, MSG_RECEIVER_STOP);
        break;
    }

    /* JL_PORTA->OUT &= ~BIT(3); */
    return 0;
}
int rf_receiver_audio_callback(u8 *rf_packet, u16 packet_len)
{
    return dongle_receiver_audio(&dongle_recv_ops, rf_packet, packet_len);
}


#endif
