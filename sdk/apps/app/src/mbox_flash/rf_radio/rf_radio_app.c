#include "rf_radio_app.h"
#include "audio_rf_mge.h"
#include "audio2rf_send.h"
#include "rf2audio_recv.h"
#include "trans_unpacket.h"
#include "trans_packet.h"
#include "app_modules.h"
#include "app.h"
#include "bsp_loop.h"
#include "hot_msg.h"

#include "cpu.h"
#include "config.h"
#include "typedef.h"
#include "vfs.h"
#include "msg.h"
#include "key.h"
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
#include "vble_complete.h"
#include "vble_simple.h"
#include "bt_ble.h"
#include "power_api.h"
#include "tick_timer_driver.h"

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[rf_radio]"
#include "log.h"

#if (RF_RADIO_EN & BLE_EN & TESTE_BLE_EN)
#define APP_SOFTOFF_CNT_TIME_10MS  3000 //3000 * 10ms
#if APP_SOFTOFF_CNT_TIME_10MS
#define app_softoff_time_reset(n) (n = maskrom_get_jiffies() + APP_SOFTOFF_CNT_TIME_10MS)
#else
#define app_softoff_time_reset(n)
#endif

#define RADIO_DEFAULT_ROLE_MASTER   0

#define RDO_EVENT_RECV_NEED_START 0
#define RDO_EVENT_RECV_NEED_STOP  1
static const u16 rdo_event2msg[] = {
    MSG_RECEIVER_START,
    MSG_RECEIVER_STOP,
};

typedef enum {
    RRA_IDLE = 0,
    RRA_STANDBY,
    RRA_ENCODING,
    RRA_DECODING,
} RRA_STATUS;

extern volatile u8 rf_radio_status;
extern volatile dec_obj *g_recv_dec_obj;
/* static u32 rf_recv_cnt AT(.rf_radio_data); */

typedef struct __radio_mge_struct {
    rev_fsm_mge rf_radio_recv_ops;
    RF_RADIO_ENC_HEAD enc_head;
    volatile u32 rdo_event_status;
    u32 app_softoff_jif_cnt;
    u32 app_standby_jif_cnt;
    volatile u8 rra_status;
    volatile u8 rra_event_occur;
} radio_mge_struct;
static radio_mge_struct radio_mge AT(.rf_radio_data);

/*----------------------------------------------------------------------------*/
/**@brief   对讲机应用事件收发处理
   @param
   @return
   @author
   @note
*/
/*----------------------------------------------------------------------------*/
static void clear_rdo_event(u32 event, u32 *event_buf)
{
    if (event >= ARRAY_SIZE(rdo_event2msg)) {
        return;
    }
    OS_ENTER_CRITICAL();
    *event_buf &= ~BIT(event % 32);
    OS_EXIT_CRITICAL();
}
static void post_rdo_event(u32 event, u32 *event_buf)
{
    if (event >= ARRAY_SIZE(rdo_event2msg)) {
        return;
    }
    log_info(">>> evenr post : 0x%x\n", event);
    OS_ENTER_CRITICAL();
    *event_buf |= BIT(event % 32);
    OS_EXIT_CRITICAL();
}
static u32 get_rdo_event(u32 event_buf)
{
    OS_ENTER_CRITICAL();
    u32 event_cls;
    __asm__ volatile("%0 = clz(%1)":"=r"(event_cls):"r"(event_buf));
    if (event_cls != 32) {
        OS_EXIT_CRITICAL();
        log_info(" has event 0x%x\n", (31 - event_cls));
        return (31 - event_cls);
    }
    OS_EXIT_CRITICAL();
    return NO_EVENT;
}
/*----------------------------------------------------------------------------*/
/**@brief   对讲机应用接收数据分发处理
   @param   ops         :状态机管理句柄
            buff        :本次接收数据buff
            packet_len  :本次接收数据长度
   @return
   @author
   @note
*/
/*----------------------------------------------------------------------------*/
static int rf_receiver_deal_phy(rev_fsm_mge *ops, u8 *buff, u16 packet_len)
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
        memcpy(&radio_mge.enc_head, (void *)pdata, sizeof(radio_mge.enc_head));
        post_rdo_event(RDO_EVENT_RECV_NEED_START, &radio_mge.rdo_event_status);
        break;
    case AUDIO2RF_DATA_PACKET:
        if (0 == if_decoder_is_run(g_recv_dec_obj)) {
            post_rdo_event(RDO_EVENT_RECV_NEED_START, &radio_mge.rdo_event_status);
            break;
        }
        rf2audio_receiver_write_data(pdata, ops->length);
        /* if (rf_recv_cnt == ops->packet_index - 1) { */
        /*     rf_recv_cnt = ops->packet_index; */
        /* } else { */
        /* log_info("************ Recv index wrong!!! %d / %d\n", rf_recv_cnt, ops->packet_index); */
        /* } */
        break;
    case AUDIO2RF_STOP_PACKET:
        log_info("************ RX PACK_STOP ************\n");
        /* rf_recv_cnt = -1; */
        memset(&radio_mge.enc_head, 0, sizeof(radio_mge.enc_head));
        post_rdo_event(RDO_EVENT_RECV_NEED_STOP, &radio_mge.rdo_event_status);
        break;
    }

    /* JL_PORTA->OUT &= ~BIT(3); */
    return 0;
}

static int rf_receiver_deal(u8 *rf_packet, u16 packet_len)
{
    radio_mge.rra_event_occur = 1;
    return rf_receiver_deal_phy(&radio_mge.rf_radio_recv_ops, rf_packet, packet_len);
}

/*----------------------------------------------------------------------------*/
/**@brief   对讲机应用无线传输发送
   @param   data:发送的数据buff
            len :发送的数据长度
   @return  0:成功   非0:失败，错误值请查看errno-base.h
   @author
   @note
*/
/*----------------------------------------------------------------------------*/
static int rf_radio_send_api(u8 *data, u16 len)
{
    /* JL_PORTA->DIR &= ~BIT(2); */
    /* JL_PORTA->OUT |=  BIT(2); */
    int ret = 0;
    /* #if TRANS_DATA_HID_EN */
    /*     ret = vble_slave_send_api(ATT_SLV2MSTR_RF_RADIO_IDX, data, len); */
    /* #elif TRANS_DATA_SPPLE_EN */
    /*     ret = vble_master_send_api(ATT_MSTR2SLV_RF_RADUI_IDX, data, len); */
    /* #elif TESTE_BLE_EN */
    ret = vble_smpl_send_api(data, len);
    /* #endif */
    /* JL_PORTA->OUT &= ~BIT(2); */
    return ret;
}
/*----------------------------------------------------------------------------*/
/**@brief   对讲机应用无线传输状态查询
   @param
   @return  1:状态正常   0:状态异常
   @author
   @note
*/
/*----------------------------------------------------------------------------*/
static int rf_radio_check_status_api(void)
{
    int ble_status;
    vble_smpl_ioctl(VBLE_SMPL_GET_STATUS, &ble_status);
    if ((ble_status == BLE_ST_NOTIFY_IDICATE) || (ble_status == BLE_ST_SEARCH_COMPLETE)) {
        return 1;
    } else {
        return 0;
    }
}
/*----------------------------------------------------------------------------*/
/**@brief   对讲机应用无线传输可发送长度查询
   @param
   @return  当前无线传输模块可发送的数据长度
   @author
   @note
*/
/*----------------------------------------------------------------------------*/
static int rf_radio_get_valid_len_api(void)
{
    return get_buffer_vaild_len(0);
}

const audio2rf_send_mge_ops rf_radio_ops = {
    .send = rf_radio_send_api,
    .check_status = rf_radio_check_status_api,
    .get_valid_len = rf_radio_get_valid_len_api,
};

static u16 rf_radio_key_filter(u8 key_status, u8 key_num, u8 key_type)
{
    radio_mge.rra_event_occur = 1;
    return rf_radio_key_msg_filter(key_status, key_num, key_type);
}

static void rrapp_open(void)
{
    audio_init();
}
static void rrapp_close(void)
{
    audio_off();
}

static void rf_radio_stop_encode(ENC_STOP_WAIT wait)
{
    audio2rf_send_stop_packet();
    stop_encode_phy(wait, 0);
    audio_adc_off_api();
}
static void rf_radio_stop_decode(void)
{
    rf2audio_decoder_stop(STREAM_TO_DAC);
    dac_off_api();
}

static void rf_radio_app_init(void)
{
    /* overlay变量初始化 */
    audio_rf_clr_buf();
    memset(&radio_mge, 0, sizeof(radio_mge));
    /* rf_recv_cnt = -1; */

    /* 蓝牙BLE初始化 */
    /* #if TRANS_DATA_HID_EN */
    /*     vble_slave_init_api(); */
    /* vble_slave_recv_cb_register(ATT_MSTR2SLV_RF_RADUI_IDX, rf_receiver_deal); */
    /* #elif TRANS_DATA_SPPLE_EN */
    /*     vble_master_init_api(); */
    /*     vble_master_recv_cb_register(ATT_SLV2MSTR_RF_RADIO_IDX, rf_receiver_deal); */
    /* #elif TESTE_BLE_EN */
    ble_clock_init();
#if RADIO_DEFAULT_ROLE_MASTER
    vble_smpl_master_select();
#else
    vble_smpl_slave_select();
#endif
    vble_smpl_init();
    vble_smpl_recv_register(rf_receiver_deal);
    /* #endif */
    /* 音频传输发送接口注册 */
    audio2rf_send_register(&rf_radio_ops);
    /* audio_adc_init_api(32000, AUDIO_ADC_MIC, 0); */
    dac_off_api();
    decoder_init();
    key_table_sel(rf_radio_key_filter);
}

static void rf_radio_app_uninit(void)
{
    if (RADIO_SENDIND == rf_radio_status) {
        rf_radio_stop_encode(ENC_NO_WAIT);
    }
    rf2audio_decoder_stop(STREAM_TO_DAC);
    audio_init();
    dac_init_api(SR_DEFAULT);
    audio2rf_send_register(NULL);
    /* #if TRANS_DATA_HID_EN */
    /*     vble_slave_recv_cb_register(ATT_SLV2MSTR_RF_RADIO_IDX, NULL); */
    /* #elif TRANS_DATA_SPPLE_EN */
    /*     vble_master_recv_cb_register(ATT_SLV2MSTR_RF_RADIO_IDX, NULL); */
    /* #elif TESTE_BLE_EN */
    vble_smpl_recv_register(NULL);
    vble_smpl_exit();
    /* #endif */
    key_table_sel(NULL);
}

void rra_in_idle(void)
{
    radio_mge.rra_status = RRA_IDLE;
    app_softoff_time_reset(radio_mge.app_softoff_jif_cnt);
    radio_mge.app_standby_jif_cnt = maskrom_get_jiffies() + 10;
}

void rra_encode_stop(ENC_STOP_WAIT wait)
{
    if (RRA_ENCODING == radio_mge.rra_status) {
        log_info("msg_radio_enc_end\n");
        rf_radio_stop_encode(wait);
        set_radio_status(RADIO_IDLE);
        rra_in_idle();
    }
}
void rra_decode_stop(void)
{
    if (RRA_DECODING == radio_mge.rra_status) {
        log_info("msg_radio_dec_end\n");
        rf_radio_stop_decode();
        set_radio_status(RADIO_IDLE);
        rra_in_idle();
    }
}

/*----------------------------------------------------------------------------*/
/**@brief   对讲机活跃状态应用
   @param   active_msg: standby下获取到需要响应的消息
   @return
   @author
   @note    活跃状态下响应应用消息和事件，应用功能空闲后回到standby状态
**/
/*----------------------------------------------------------------------------*/
bool rrapp_run(int active_msg)
{
    log_info("rf_radio run\n");

    int msg[2], err, ble_status;

    u32 event, res_flag;

    if (NO_MSG != active_msg) {
        msg[0] = active_msg;
        goto __rrapp_msg_deal;
    }

    while (1) {

        if (RRA_IDLE == radio_mge.rra_status) {
#if APP_SOFTOFF_CNT_TIME_10MS
            if (time_after(maskrom_get_jiffies(), radio_mge.app_softoff_jif_cnt)) {
                post_msg(1, MSG_POWER_OFF);
            }
#endif
            if (time_after(maskrom_get_jiffies(), radio_mge.app_standby_jif_cnt)) {
                vble_smpl_ioctl(VBLE_SMPL_GET_STATUS, &ble_status);
                if ((ble_status == BLE_ST_NOTIFY_IDICATE)/*从机已连接并可发数*/ || \
                    (ble_status == BLE_ST_SEARCH_COMPLETE)/*主机已连接并可发数*/) {
                    radio_mge.rra_status = RRA_STANDBY;
                    return true;
                }
            }
        }

        bsp_loop();

        OS_ENTER_CRITICAL();
        event = get_rdo_event(radio_mge.rdo_event_status);
        if (event != NO_EVENT) {
            clear_rdo_event(event, &radio_mge.rdo_event_status);
            msg[0] = rdo_event2msg[event];
            OS_EXIT_CRITICAL();
        } else {
            OS_EXIT_CRITICAL();
            err = get_msg(2, &msg[0]);
            if (MSG_NO_ERROR != err) {
                msg[0] = NO_MSG;
                log_info("get msg err 0x%x\n", err);
            }
        }

__rrapp_msg_deal:
        switch (msg[0]) {
        case MSG_SENDER_START:
            vble_smpl_ioctl(VBLE_SMPL_GET_STATUS, &ble_status);
            if ((ble_status == BLE_ST_NOTIFY_IDICATE)/*从机已连接并可发数*/ || \
                (ble_status == BLE_ST_SEARCH_COMPLETE)/*主机已连接并可发数*/) {
                local_irq_disable();
                if (RADIO_IDLE != rf_radio_status) {
                    local_irq_enable();
                    break;
                }
                set_radio_status(RADIO_SENDIND);
                local_irq_enable();
                log_info("MSG_RADIO_START\n");
                vble_smpl_ioctl(VBLE_SMPL_UPDATE_CONN_INTERVAL, 8);
                audio_adc_init_api(32000, AUDIO_ADC_MIC, 0);
#if ENCODER_UMP3_EN
                res_flag = audio2rf_encoder_io(ump3_encode_api, enc_input, rf_enc_output, INDEX_UMP3);
#endif
#if ENCODER_OPUS_EN
                /* res_flag = audio2rf_encoder_io(opus_encode_api, enc_input, rf_enc_output, INDEX_OPUS); */
#endif
                if (res_flag) {
                    radio_mge.rra_status = RRA_ENCODING;
                } else {
                    rra_encode_stop(ENC_NO_WAIT);
                    vble_smpl_ioctl(VBLE_SMPL_UPDATE_CONN_INTERVAL, 40);
                }
            } else {
                log_info("Radio isn't Ready! status:%d %d\n", rf_radio_status, ble_status);
            }
            break;
        case MSG_SENDER_STOP:
            if (RADIO_SENDIND != rf_radio_status) {
                break;
            }
            rra_encode_stop(ENC_NEED_WAIT);
            vble_smpl_ioctl(VBLE_SMPL_UPDATE_CONN_INTERVAL, 40);
            break;
        case MSG_WFILE_FULL:
        case MSG_SENDER_STOP_NOW:
            if (RADIO_SENDIND != rf_radio_status) {
                break;
            }
            log_info("MSG_SENDER_STOP_NOW\n");
            rra_encode_stop(ENC_NO_WAIT);
            break;
        case MSG_RECEIVER_START:
            if (RADIO_RECEIVING == rf_radio_status) {
                break;
            } else if (RADIO_SENDIND == rf_radio_status) {
                char *role_name;
                vble_smpl_ioctl(VBLE_SMPL_GET_NAME, (int)&role_name);
                /* log_info("rname:%s\n", role_name); */
                if (0 == strcmp(role_name, "ble_slave")) {
                    /* 主从同时发送，优先响应主机发送，从机退出发送转为接收 */
                    rra_encode_stop(ENC_NO_WAIT);
                } else {
                    break;
                }
            }
            log_info("MSG_RECEIVER_START\n");
            RF_RADIO_ENC_HEAD *p_enc_head = &radio_mge.enc_head;
            if (p_enc_head->reserved == 0xff) {
                dac_init(SR_DEFAULT, 0);
                res_flag = rf2audio_decoder_start(p_enc_head, STREAM_TO_DAC, SR_DEFAULT);
                if (res_flag) {
                    radio_mge.rra_status = RRA_DECODING;
                    set_radio_status(RADIO_RECEIVING);
                } else {
                    rra_decode_stop();
                }
            }
            break;
        case MSG_RECEIVER_STOP:
            if (RADIO_RECEIVING != rf_radio_status) {
                break;
            }
            log_info("MSG_RECEIVER_STOP\n");
            rra_decode_stop();
            break;
        case MSG_BLE_ROLE_SWITCH:
            if (RADIO_IDLE != rf_radio_status) {
                break;
            }
            log_info("MSG_BLE_ROLE_SWITCH\n");
            vble_smpl_recv_register(NULL);
            vble_smpl_exit();
            vble_smpl_switch_role(); //主从内部切换
            vble_smpl_init();
            vble_smpl_recv_register(rf_receiver_deal);
            break;
        case MSG_CHANGE_WORK_MODE:
            log_info("MSG_CHANGE_WORK_MODE\n");
            return false;
        case MSG_500MS:
            log_char('5');
        default:
            ap_handle_hotkey(msg[0]);
            break;
        }
    }
    return true;
}

/*----------------------------------------------------------------------------*/
/**@brief   对讲机standby低功耗应用
   @param
   @return  需要退出standby响应的消息
   @author
   @note    蓝牙已连接且应用空闲时系统进入standby状态，在蓝牙通信间隙进行pdown休眠
            当有连接、系统、应用事件触发时，退出standby状态
*/
/*----------------------------------------------------------------------------*/
static int rrapp_exit_standby(int msg)
{
    rra_in_idle();
    rrapp_open();
    return msg;
}
int rrapp_standby(void)
{
    /* log_info("rf_radio idle, sf_to:%d\n", radio_mge.app_softoff_jif_cnt - maskrom_get_jiffies()); */
    rrapp_close();
    radio_mge.rra_event_occur = 0;

    bool has_event = 0;
    int ble_status, err, msg[2];
    msg[0] = NO_MSG;
    while (1) {
        sys_power_down(2000000);
        wdt_clear();
#if APP_SOFTOFF_CNT_TIME_10MS
        /* 判断定时关机 */
        if (time_after(maskrom_get_jiffies(), radio_mge.app_softoff_jif_cnt)) {
            return rrapp_exit_standby(MSG_POWER_OFF);
        }
#endif

        vble_smpl_ioctl(VBLE_SMPL_GET_STATUS, &ble_status);
        if ((ble_status != BLE_ST_NOTIFY_IDICATE)/*从机已连接并可发数*/ && \
            (ble_status != BLE_ST_SEARCH_COMPLETE)/*主机已连接并可发数*/) {
            /* 断开连接，退出idle回到活跃状态等待下一次连接 */
            log_info("ble conn status :%d\n", ble_status);
            return rrapp_exit_standby(NO_MSG);
        }

        has_event = has_sys_event(); //系统事件发生，退出idle
        if (has_event) {
            log_info("has sys_event\n");
            return rrapp_exit_standby(NO_MSG);
        }
        if (radio_mge.rra_event_occur) { //应用事件发生，退出idle
            log_info("rf_radio event occur\n");
            radio_mge.rra_event_occur = 0;
            return rrapp_exit_standby(NO_MSG);
        }

        err = get_msg_phy(2, &msg[0], 0);//获取到指定消息，退出idle
        switch (msg[0]) {
        /* 需要响应的消息 */
        /* case MSG_POWER_OFF: */
        /*     log_info("has msg:0x%x\n", msg[0]); */
        /*     return rrapp_exit_standby(msg[0]); */

        /* 不需要响应的消息 */
        case MSG_500MS:
        default:
            msg[0] = NO_MSG;
            break;
        }
    }
    return rrapp_exit_standby(msg[0]);
}

/*----------------------------------------------------------------------------*/
/**@brief   对讲机应用主函数
   @param
   @return
   @author
   @note    主函数根据不同状态运行不同子函数；
            蓝牙已连接且应用空闲时，执行standby函数低功耗保持连接
            当外部事件触发时退出standby，运行run函数响应消息、事件
* /
/*----------------------------------------------------------------------------*/
void rf_radio_app(void)
{
    int msg = NO_MSG;

    u32 flag;

    rf_radio_app_init();

    set_radio_status(RADIO_IDLE);

    rra_in_idle();

    rrapp_open();

    while (1) {
        if (RRA_STANDBY == radio_mge.rra_status) {
            /* 应用空闲状态 */
            msg = rrapp_standby();
        } else {
            /* 应用工作状态 */
            flag = rrapp_run(msg);
            if (!flag) {
                goto __rf_radio_app_exit;
            }
        }

    }
__rf_radio_app_exit:
    rf_radio_app_uninit();
    return;
}
#endif
