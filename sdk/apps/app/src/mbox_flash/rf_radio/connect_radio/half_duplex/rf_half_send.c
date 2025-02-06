#include "audio_adc_api.h"
#include "encoder_stream.h"
#include "audio2rf_send.h"

#include "vble_simple.h"
#include "trans_unpacket.h"
#include "tick_timer_driver.h"
#include "jiffies.h"
#include "msg.h"
#include "wdt.h"
#include "hci_ll.h"

#include "connect_radio.h"
#include "rf_half_duplex.h"
#include "rf_radio_app.h"

#include "app_modules.h"
#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[rf_half_send]"
#include "log.h"

#if (RF_RADIO_EN & BLE_EN & TESTE_BLE_EN)

//----
extern connect_rf_mge g_half_rf_mge;
extern rev_fsm_mge    g_half_packet;
//----
static enc_obj *sp_half_enc_obj;


static void rra_encode_start(void)
{
    audio2rf_encoder_start((enc_obj *)sp_half_enc_obj);     //audio_adc使能
    g_half_rf_mge.enc_status = RRA_ENCODING;
}

static bool rfr_half_encode_stop(IS_WAIT enc_wait)
{
    if (NULL != sp_half_enc_obj) {
        sp_half_enc_obj = (enc_obj *)audio2rf_encoder_stop(sp_half_enc_obj, enc_wait, NEED_WAIT);
        delay_10ms(2);
        rf_queue_uninit(&g_half_rf_mge.speaker_queue, NEED_WAIT);
    }
    g_half_rf_mge.enc_status = RRA_ENC_IDLE;  //空闲状态i
    return true;
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
    rev_fsm_mge *p_recv_ops = &g_half_packet;
    int connect_handle;
    u8 rra_packet[16] ALIGNED(2);        //所有对讲机
    u32 len = packet_cmd_get(&p_recv_ops->cmd_cbuf, &rra_packet[0], sizeof(rra_packet));
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
            rra_send_ack_cmd(&g_half_rf_mge.listener_queue, AUDIO2RF_START_PACKET, 0);
        }
        break;

    case (AUDIO2RF_ACK | AUDIO2RF_START_PACKET):
        log_info("AUDIO2RF_START_PACKET_ACK");
        u8 *cmd_data = &rra_packet[2];
        if (g_half_rf_mge.enc_status == RRA_ENC_WAITING_START_ACK) { //等待对方回应ACK状态
            if (1 == cmd_data[0]) {
                log_info("real encode start\n");
                vble_smpl_ioctl(VBLE_SMPL_GET_CONN_HANDLE, (int)&connect_handle);
                if (0 != connect_handle) {
                    ll_vendor_rx_max(connect_handle, ACL_RTX_DATA_LEN); //减少蓝牙最大接收数据长度以提高蓝牙接收数据成功率
                }
                rra_encode_start();
            } else if (2 == cmd_data[0]) {
                audio2rf_stop_cmd(&g_half_rf_mge.speaker_queue);
            } else {
                log_info("receive need stop\n");
                rfr_half_encode_stop(NO_WAIT);
                return true;
            }
        }
        break;
    }
    return false;
}


void rrapp_sending(int active_msg)
{
    int msg[2], err, ble_status;
    u32 event, retry;
    u32 interval_target_jiff, ack_target_jiff;
    u32 t_interval;

    sp_half_enc_obj = NULL;
    g_half_rf_mge.enc_status = RRA_ENC_IDLE;  //空闲状态
    audio_adc_init_api(RA_ENC_SR, AUDIO_ADC_MIC, 0);


    if (NO_MSG != active_msg) {
        msg[0] = active_msg;
        active_msg = NO_MSG;
        goto __rrapp_sending_msg_deal;
    }


    while (1) {
        switch (g_half_rf_mge.enc_status) { //判断对讲机状态
        case RRA_ENC_IDLE:  //空闲状态
            vble_smpl_ioctl(VBLE_SMPL_UPDATE_CONN_INTERVAL, RADIO_WORKING_INTERVAL);
            interval_target_jiff = maskrom_get_jiffies() + 10;   //如果初始化成功，则设置10ms重发ack ????
            retry = 5;
            g_half_rf_mge.enc_status = RRA_ENC_STANDBY;   //空闲状态 */
            break;

        case RRA_ENC_STANDBY:
            t_interval =  get_curr_true_interval();
            if (t_interval == RADIO_WORKING_INTERVAL) {     //第一次更新参数成功了 且 为工作interval
                log_info("RRA_ENC_STANDBY to RRA_ENC_WAITING_START_ACK\n");
                sp_half_enc_obj = audio2rf_encoder_io(\
                                                      RA_ENC_FUNC,                      \
                                                      (void *)&g_half_rf_mge.speaker_queue, \
                                                      (void *)&rf_radio_ops);
                //--
                if (NULL == sp_half_enc_obj) {
                    /* rfr_half_encode_stop(NO_WAIT); */
                    goto __rrapp_sending_exit;
                }

                audio2rf_start_cmd(\
                                   &g_half_rf_mge.speaker_queue,   \
                                   sp_half_enc_obj->info.sr, \
                                   sp_half_enc_obj->info.br, \
                                   RA_ENC_TYPE);  //发送ACK给对方

                g_half_rf_mge.enc_status = RRA_ENC_WAITING_START_ACK;   //设置10ms重发ACK
                ack_target_jiff = maskrom_get_jiffies() + 5;
                retry = 5;
            } else {
                if (time_after(maskrom_get_jiffies(), interval_target_jiff)) {
                    interval_target_jiff = maskrom_get_jiffies() + 10;
                    if (0 == retry--) {
                        goto __rrapp_sending_exit;
                    }
                }
            }
            break;

        case RRA_ENC_WAITING_START_ACK: //等待对方回应ACK状态
            if (time_after(maskrom_get_jiffies(), ack_target_jiff)) {   //10ms内没有受到回应ACK
                log_info("resending start packet!\n");

                audio2rf_start_cmd(\
                                   &g_half_rf_mge.speaker_queue,    \
                                   sp_half_enc_obj->info.sr,  \
                                   sp_half_enc_obj->info.br,  \
                                   RA_ENC_TYPE);  //10ms再次重发

                if (0 == retry--) { //5次都没收到，退出本次发送
                    log_info("radio_start fail!\n");
                    goto __rrapp_sending_exit;
                }

                ack_target_jiff = maskrom_get_jiffies() + 5;
            }
        /* break; */
        default:
            t_interval = get_curr_true_interval();
            if (0 != t_interval) {
                if (t_interval != RADIO_WORKING_INTERVAL) {
                    if (
                        (RRA_ENCODING == g_half_rf_mge.enc_status) || \
                        (NULL != sp_half_enc_obj) \
                    ) {
                        cbuf_clear(g_half_rf_mge.speaker_queue.cbuf);
                        rfr_half_encode_stop(NO_WAIT);
                    }
                    g_half_rf_mge.enc_status = RRA_ENC_IDLE;  //空闲状态
                } else {
                    log_error("multile same interval update");
                }
            }

            break;

        }


        event = radio_rf_sending_loop();
        if (true == event) {
            log_info("true_event_return\n");
            goto __rrapp_sending_exit;
        }

        err = get_msg(2, &msg[0]);
        if (MSG_NO_ERROR != err) {
            msg[0] = NO_MSG;
        }


__rrapp_sending_msg_deal:
        switch (msg[0]) {
        case MSG_SENDER_STOP:
            rfr_half_encode_stop(NEED_WAIT);
            goto __rrapp_sending_exit;
        case MSG_WFILE_FULL:
        case MSG_SENDER_STOP_NOW:
            log_info("MSG_SENDER_STOP_NOW\n");
            goto __rrapp_sending_exit;
        case MSG_500MS:
            wdt_clear();
            vble_smpl_ioctl(VBLE_SMPL_GET_STATUS, (int)&ble_status);
            if ((ble_status != BLE_ST_NOTIFY_IDICATE)/*从机断开*/ && \
                (ble_status != BLE_ST_SEARCH_COMPLETE)/*主机断开*/) {
                goto __rrapp_sending_exit;
            }
            break;
        default:
            break;
        }
    }
__rrapp_sending_exit:
    rfr_half_encode_stop(NO_WAIT);
    audio_adc_off_api();
    g_half_rf_mge.enc_status = RRA_ENC_IDLE;  //空闲状态
    g_rfr_half_mode = RFR_HALF_RECIVE_MODE;
}


#endif
