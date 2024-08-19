#include "rf_radio_app.h"
#include "rf_radio_rs.h"
#include "audio_adc_api.h"
#include "encoder_stream.h"

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[rf_radio]"
#include "log.h"

#if (RF_RADIO_EN & BLE_EN & TESTE_BLE_EN)

enc_obj *rra_encode_init(void)
{
    log_info("rra_enc_init\n");
    if ((RRA_DEC_IDLE == radio_mge.rra_dec_status) && (RRA_ENC_IDLE == radio_mge.rra_enc_status)) {
        vble_smpl_ioctl(VBLE_SMPL_UPDATE_CONN_INTERVAL, RADIO_WORKING_INTERVAL);
    }
    audio_adc_init_api(RA_ENC_SR, AUDIO_ADC_MIC, 0);
    return audio2rf_encoder_io(RA_ENC_FUNC, RA_ENC_TYPE);
}
void rra_encode_start(void)
{
    log_info("rra_enc_start\n");
    rf_send_soft_isr_init(radio_mge.enc_obj->p_obuf, 1);
    audio2rf_encoder_start((enc_obj *)radio_mge.enc_obj);
    radio_mge.rra_enc_status = RRA_ENCODING;
}
void rra_encode_stop(ENC_STOP_WAIT wait)
{
    log_info("rra_enc_stop %d\n", wait);
    audio2rf_encoder_stop(wait);
    rf_send_soft_isr_init(NULL, 1);
    radio_mge.enc_obj = NULL;
    audio_adc_off_api();
    rra_tx_in_idle();
}



/*----------------------------------------------------------------------------*/
/**@brief   对讲机发送状态处理
   @param
   @return
   @author
   @note    发送状态下响应应用消息和事件，应用功能空闲后回到空闲状态
**/
/*----------------------------------------------------------------------------*/
static bool radio_rf_sending_loop(void)
{
    rev_fsm_mge *p_recv_ops = &radio_mge.packet_recv;
    u32 len = packet_cmd_get(p_recv_ops->cmd_pool, &rra_packet[0], sizeof(rra_packet));
    if (0 == len) {
        return false;
    }
    switch (rra_packet[1]) {
    case AUDIO2RF_START_PACKET:
        /* 发送时收到start命令，优先响应主机发送，从机由发送转为接收 */
        log_info("AUDIO2RF_START_PACKET");
        char *role_name;
        vble_smpl_ioctl(VBLE_SMPL_GET_NAME, (int)&role_name);
        log_info("rname:%s\n", role_name);
        if (0 == strcmp(role_name, "ble_master")) {
            rra_send_ack_cmd(AUDIO2RF_START_PACKET, 0);
        }
        break;

    case (AUDIO2RF_ACK | AUDIO2RF_START_PACKET):
        log_info("AUDIO2RF_START_PACKET_ACK");
        u8 *cmd_data = &rra_packet[2];
        if (1 == cmd_data[0]) {
            rra_encode_start();
        } else {
            rra_encode_stop(ENC_NO_WAIT);
            return true;
        }
        break;
    }
    return false;
}
bool rrapp_sending(int active_msg)
{
    int msg[2], err, ble_status;
    u32 event, retry, target_jiff;

    rrapp_send_queue_init();

    radio_mge.enc_obj = rra_encode_init();
    if (NULL != radio_mge.enc_obj) {
        audio2rf_start_cmd(radio_mge.enc_obj->info.sr, radio_mge.enc_obj->info.br, RA_ENC_TYPE);
        radio_mge.rra_enc_status = RRA_ENC_WAITING_START_ACK;
        target_jiff = maskrom_get_jiffies() + 5;
        retry = 5;
    } else {
        audio_adc_off_api();
        rra_tx_in_idle();
        goto __rrapp_sending_exit;
    }
    if (NO_MSG != active_msg) {
        msg[0] = active_msg;
        goto __rrapp_sending_msg_deal;
    }
    while (1) {

        if (RRA_ENC_WAITING_START_ACK == radio_mge.rra_enc_status) {
            if (time_after(maskrom_get_jiffies(), target_jiff)) {
                log_info("resending start packet!\n");
                audio2rf_start_cmd(radio_mge.enc_obj->info.sr, radio_mge.enc_obj->info.br, RA_ENC_TYPE);
                target_jiff = maskrom_get_jiffies() + 5;
                if (0 == retry--) {
                    log_info("radio_start fail!\n");
                    rra_encode_stop(ENC_NO_WAIT);
                    goto __rrapp_sending_exit;
                }
            }
        }

        err = get_msg(2, &msg[0]);
        if (MSG_NO_ERROR != err) {
            msg[0] = NO_MSG;
            log_info("get msg err 0x%x\n", err);
        }

        event = radio_rf_sending_loop();
        if (true == event) {
            log_info("true_event_return\n");
            goto __rrapp_sending_exit;
        }

__rrapp_sending_msg_deal:
        switch (msg[0]) {
        case MSG_SENDER_STOP:
            log_info("MSG_SENDER_STOP\n");
            rra_encode_stop(ENC_NEED_WAIT);
            goto __rrapp_sending_exit;
        case MSG_WFILE_FULL:
        case MSG_SENDER_STOP_NOW:
            log_info("MSG_SENDER_STOP_NOW\n");
            rra_encode_stop(ENC_NO_WAIT);
            goto __rrapp_sending_exit;
        case MSG_500MS:
            wdt_clear();
            vble_smpl_ioctl(VBLE_SMPL_GET_STATUS, (int)&ble_status);
            if ((ble_status != BLE_ST_NOTIFY_IDICATE)/*从机断开*/ && \
                (ble_status != BLE_ST_SEARCH_COMPLETE)/*主机断开*/) {
                rra_encode_stop(ENC_NO_WAIT);
                goto __rrapp_sending_exit;
            }
            break;
        default:
            break;
        }
    }
__rrapp_sending_exit:
    radio_mge.rra_mode = RRA_RECIVE_MODE;
    return true;
}


#endif
