
#include "rf_radio_app.h"
#include "rf_radio_rs.h"
#include "trans_packet.h"
#include "key.h"
#include "audio.h"
#include "power_api.h"
#include "rf_pa_port.h"
#include "hot_msg.h"
#include "audio_adc_api.h"

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[rf_radio]"
#include "log.h"

static u32 fd_rrapp_receiving_loop(void)
{
    rev_fsm_mge *p_recv_ops = &radio_mge.packet_recv;
    u32 len = packet_cmd_get(p_recv_ops->cmd_pool, &rra_packet[0], sizeof(rra_packet));
    if (0 == len) {
        return E_PACKET_NULL;
    }
    switch (rra_packet[1]) {
    case AUDIO2RF_START_PACKET:
        log_info("DEC_AUDIO2RF_START_PACKET");
        if (RRA_DECODING == radio_mge.rra_dec_status) {
            rra_send_ack_cmd(AUDIO2RF_START_PACKET, 1);
            break;
        }
        RF_RADIO_ENC_HEAD *p_enc_head = (RF_RADIO_ENC_HEAD *)&rra_packet[2];
        radio_mge.packet_recv.dec_obj = rra_decode_start(p_enc_head);
        if (NULL != radio_mge.packet_recv.dec_obj) {
            rra_send_ack_cmd(AUDIO2RF_START_PACKET, 1);
        } else {
            rra_decode_stop();
            rra_send_ack_cmd(AUDIO2RF_START_PACKET, 0);
        }
        break;
    case AUDIO2RF_STOP_PACKET:
        log_info("STOP_AUDIO2RF_START_PACKET");
        rra_decode_stop();
        break;
    case (AUDIO2RF_ACK | AUDIO2RF_START_PACKET):
        log_info("ACK_AUDIO2RF_START_PACKET_ACK");
        u8 *cmd_data = &rra_packet[2];
        if ((1 == cmd_data[0]) && (RRA_ENC_WAITING_START_ACK == radio_mge.rra_enc_status)) {
            rra_encode_start();
        } else {
            rra_encode_stop(ENC_NO_WAIT);
            rrapp_send_queue_init();
            return 0;
        }
        break;
    }
    return 0;
}

void fd_radio_decode_idle()
{
    radio_mge.rra_dec_status = RRA_DEC_IDLE;
    app_softoff_time_reset(radio_mge.app_softoff_jif_cnt);
    radio_mge.app_standby_jif_cnt = maskrom_get_jiffies() + 10;
}
void fd_radio_encode_idle()
{
    radio_mge.rra_enc_status = RRA_ENC_IDLE;
    app_softoff_time_reset(radio_mge.app_softoff_jif_cnt);
    radio_mge.app_standby_jif_cnt = maskrom_get_jiffies() + 10;
}

/*----------------------------------------------------------------------------*/
/**@brief   全双工对讲机应用主函数
   @param
   @return
   @author
   @note    全双工对讲机在工作模式下会进入该函数；
            在连接成功后会退出该函数进入idle模式，以低功耗模式待机
*/
/*----------------------------------------------------------------------------*/
u8 fd_rrapp_loop(int active_msg)
{
    int msg[2], err, ble_status;
    u32 event, retry, target_jiff;

    rrapp_send_queue_init();
    if (NO_MSG != active_msg) {
        msg[0] = active_msg;
        goto __fd_rrapp_msg_deal;
    }
    while (1) {

        if ((RRA_DEC_IDLE == radio_mge.rra_dec_status) && (RRA_ENC_IDLE == radio_mge.rra_enc_status)) {
#if APP_SOFTOFF_CNT_TIME_10MS
            if (time_after(maskrom_get_jiffies(), radio_mge.app_softoff_jif_cnt)) {
                post_msg(1, MSG_POWER_OFF);
            }
#endif
            if (time_after(maskrom_get_jiffies(), radio_mge.app_standby_jif_cnt)) {
                vble_smpl_ioctl(VBLE_SMPL_GET_STATUS, (int)&ble_status);
                if ((ble_status == BLE_ST_NOTIFY_IDICATE)/*从机已连接并可发数*/ || \
                    (ble_status == BLE_ST_SEARCH_COMPLETE)/*主机已连接并可发数*/) {
                    radio_mge.rra_mode = RRA_IDLE_MODE;
                    goto __exit_fd_rrapp_loop;
                }
            }
        }
        err = get_msg(2, &msg[0]);
        if (MSG_NO_ERROR != err) {
            msg[0] = NO_MSG;
            log_info("get msg err 0x%x\n", err);
        }

        if (RRA_ENC_WAITING_START_ACK == radio_mge.rra_enc_status) {
            if (time_after(maskrom_get_jiffies(), target_jiff)) {
                log_info("resending start packet!\n");
                audio2rf_start_cmd(radio_mge.enc_obj->info.sr, radio_mge.enc_obj->info.br, RA_ENC_TYPE);
                target_jiff = maskrom_get_jiffies() + 10;
                if (0 == retry--) {
                    log_info("radio_start fail!\n");
                    rra_encode_stop(ENC_NO_WAIT);
                    rrapp_send_queue_init();
                }
            }
        }

        fd_rrapp_receiving_loop();


__fd_rrapp_msg_deal:
        switch (msg[0]) {
        case MSG_SENDER_START:
            if ((RRA_ENCODING == radio_mge.rra_enc_status) || (RRA_ENC_WAITING_START_ACK == radio_mge.rra_enc_status)) {
                break;
            }
            vble_smpl_ioctl(VBLE_SMPL_GET_STATUS, (int)&ble_status);
            if ((ble_status == BLE_ST_NOTIFY_IDICATE)/*从机已连接并可发数*/ || \
                (ble_status == BLE_ST_SEARCH_COMPLETE)/*主机已连接并可发数*/) {

                radio_mge.enc_obj = rra_encode_init();
                if (NULL != radio_mge.enc_obj) {
                    audio2rf_start_cmd(radio_mge.enc_obj->info.sr, radio_mge.enc_obj->info.br, RA_ENC_TYPE);
                    radio_mge.rra_enc_status = RRA_ENC_WAITING_START_ACK;
                    target_jiff = maskrom_get_jiffies() + 5;
                    retry = 5;
                } else {
                    audio_adc_off_api();
                    rra_in_idle();
                }
            } else {
                log_info("Radio isn't Ready! status:%d %d\n", radio_mge.rra_dec_status, ble_status);
            }
            break;
        case MSG_SENDER_STOP:
            log_info("MSG_SENDER_STOP\n");
            rra_encode_stop(ENC_NEED_WAIT);
            rrapp_send_queue_init();
            break;
        case MSG_WFILE_FULL:
        case MSG_SENDER_STOP_NOW:
            log_info("MSG_SENDER_STOP_NOW\n");
            rra_encode_stop(ENC_NO_WAIT);
            break;
        case MSG_BLE_ROLE_SWITCH:
            log_info("MSG_BLE_ROLE_SWITCH\n");
            if ((RRA_DEC_IDLE != radio_mge.rra_dec_status) || (RRA_ENC_IDLE != radio_mge.rra_enc_status)) {
                break;
            }
            vble_smpl_recv_register(NULL, NULL);
            vble_smpl_exit();
            vble_smpl_switch_role(); //主从内部切换
            vble_smpl_init();
            vble_smpl_recv_register(&radio_mge.packet_recv, (int (*)(void *, u8 *, u16))unpack_data_deal);
            break;
        case MSG_CHANGE_WORK_MODE:
            log_info("MSG_CHANGE_WORK_MODE\n");
            radio_mge.rra_mode = RRA_EXIT;
            goto __exit_fd_rrapp_loop;
        case MSG_500MS:
            wdt_clear();
            vble_smpl_ioctl(VBLE_SMPL_GET_STATUS, (int)&ble_status);
            if ((ble_status != BLE_ST_NOTIFY_IDICATE)/*从机断开*/ && \
                (ble_status != BLE_ST_SEARCH_COMPLETE)/*主机断开*/) {
                if (RRA_DECODING == radio_mge.rra_dec_status) {
                    rra_decode_stop();
                }
                if (RRA_ENCODING == radio_mge.rra_enc_status) {
                    log_info("disconn stop enc\n");
                    rra_encode_stop(ENC_NO_WAIT);
                    rrapp_send_queue_init();
                }
            }

        default:
            ap_handle_hotkey(msg[0]);
            break;
        }
    }

__exit_fd_rrapp_loop:
    return radio_mge.rra_mode;
}
