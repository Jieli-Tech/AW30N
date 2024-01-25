#include "rf_radio_app.h"
#include "rf_radio_rs.h"
#include "hot_msg.h"

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[rf_radio]"
#include "log.h"

#if (RF_RADIO_EN & BLE_EN & TESTE_BLE_EN)

static u32 rra_dec_ibuff[1024 * 2 / 4] AT(.rf_radio_data);

//***** for rrapp_receiving *********************

void rra_decode_stop(void)
{
    log_info("rra_dec_stop\n");
    rf2audio_decoder_stop(radio_mge.packet_recv.dec_obj, unregist_dac_channel);
    radio_mge.packet_recv.dec_obj = NULL;
    radio_mge.stream.p_ibuf = NULL;
    dac_off_api();
    rra_in_idle();
}

static dec_obj *rra_decode_start(RF_RADIO_ENC_HEAD *p_enc_head)
{
    log_info("rra_dec_init\n");
    dac_init(SR_DEFAULT, 0);
    cbuf_init(&radio_mge.dec_icbuf, &rra_dec_ibuff[0], sizeof(rra_dec_ibuff));
    memset(&radio_mge.stream, 0, sizeof(sound_stream_obj));
    radio_mge.stream.p_ibuf =  &radio_mge.dec_icbuf;
    dec_obj *p_dec_obj = rf2audio_decoder_start(p_enc_head, &radio_mge.stream, SR_DEFAULT);
    if (NULL != p_dec_obj) {
        regist_dac_channel(&p_dec_obj->sound, NULL);
        radio_mge.rra_status = RRA_DECODING;
    }
    return p_dec_obj;
}

//***** for rrapp_receiving end *********************

/*----------------------------------------------------------------------------*/
/**@brief   对讲机接收状态处理
   @param
   @return
   @author
   @note    接收状态下响应接收事件，应用功能空闲后进入standby状态
**/
/*----------------------------------------------------------------------------*/
static u32 radio_rf_receiving_loop(void)
{
    rev_fsm_mge *p_recv_ops = &radio_mge.packet_recv;
    u32 len = packet_cmd_get(p_recv_ops->cmd_pool, &rra_packet[0], sizeof(rra_packet));
    if (0 == len) {
        return E_PACKET_NULL;
    }
    switch (rra_packet[1]) {
    case AUDIO2RF_START_PACKET:
        log_info("AUDIO2RF_START_PACKET\n");
        if (RRA_DECODING == radio_mge.rra_status) {
            rra_send_ack_cmd(AUDIO2RF_START_PACKET, 1);
            break;
        }
        RF_RADIO_ENC_HEAD *p_enc_head = &rra_packet[2];
        radio_mge.packet_recv.dec_obj = rra_decode_start(p_enc_head);
        if (NULL != radio_mge.packet_recv.dec_obj) {
            /* rra_decode_start(); */
            rra_send_ack_cmd(AUDIO2RF_START_PACKET, 1);
        } else {
            rra_decode_stop();
            rra_send_ack_cmd(AUDIO2RF_START_PACKET, 0);
        }
        break;
    case AUDIO2RF_STOP_PACKET:
        log_info("AUDIO2RF_STOP_PACKET\n");
        rra_decode_stop();
        break;
    }
    return 0;
}
bool rrapp_receiving(int active_msg)
{
    int msg[2], err, ble_status;
    u32 event;
    u32 res;
    bool b_res = true;
    rrapp_send_queue_init();

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
                    b_res = true;
                    goto __exit_rrapp_receiving;
                }
            }
        }

        err = get_msg(2, &msg[0]);
        if (MSG_NO_ERROR != err) {
            msg[0] = NO_MSG;
            log_info("get msg err 0x%x\n", err);
        }

        res = radio_rf_receiving_loop();

__rrapp_msg_deal:
        switch (msg[0]) {
        case MSG_SENDER_START:
            vble_smpl_ioctl(VBLE_SMPL_GET_STATUS, &ble_status);
            if ((ble_status == BLE_ST_NOTIFY_IDICATE)/*从机已连接并可发数*/ || \
                (ble_status == BLE_ST_SEARCH_COMPLETE)/*主机已连接并可发数*/) {
                if (RRA_IDLE == radio_mge.rra_status) {
                    radio_mge.rra_status = RRA_GOTO_ENC;
                    b_res = true;
                    goto __exit_rrapp_receiving;
                }
            } else {
                log_info("Radio isn't Ready! status:%d %d\n", radio_mge.rra_status, ble_status);
            }
            break;
        case MSG_BLE_ROLE_SWITCH:
            log_info("MSG_BLE_ROLE_SWITCH\n");
            vble_smpl_recv_register(NULL, NULL);
            vble_smpl_exit();
            vble_smpl_switch_role(); //主从内部切换
            vble_smpl_init();
            vble_smpl_recv_register(&radio_mge.packet_recv, unpack_data_deal);
            break;
        case MSG_CHANGE_WORK_MODE:
            log_info("MSG_CHANGE_WORK_MODE\n");
            b_res = false;
            goto __exit_rrapp_receiving;
        case MSG_500MS:
            wdt_clear();
            if (RRA_DECODING == radio_mge.rra_status) {
                vble_smpl_ioctl(VBLE_SMPL_GET_STATUS, &ble_status);
                if ((ble_status != BLE_ST_NOTIFY_IDICATE)/*从机断开*/ && \
                    (ble_status != BLE_ST_SEARCH_COMPLETE)/*主机断开*/) {
                    rra_decode_stop();
                }
            }
        default:
            ap_handle_hotkey(msg[0]);
            break;
        }
    }
__exit_rrapp_receiving:
    if (NULL != radio_mge.packet_recv.dec_obj) {
        rra_decode_stop();
    }
    return b_res;
}

#endif
