#include "rf_radio_app.h"
#include "rf_radio_comm.h"
#include "connect_radio.h"

#include "trans_packet.h"
#include "trans_unpacket.h"
#include "rf2audio_recv.h"
#include "key.h"
#include "msg.h"
#include "jiffies.h"
#include "tick_timer_driver.h"
#include "audio_dac.h"
#include "power_api.h"
#include "hot_msg.h"
#include "audio_adc_api.h"
#include "encoder_stream.h"
#include "audio_rf_mge.h"
#include "audio2rf_send.h"
#include "vble_simple.h"
#include "hci_ll.h"

#if defined (FULL_DUPLEX_RADIO) && (FULL_DUPLEX_RADIO)

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[rf_radio_full]"
#include "log.h"

//-----
enum RFR_FULL_MODE g_rfr_full_mode;
//------
static enc_obj *sp_full_enc_obj;

static connect_rf_mge  g_full_rf_mge    AT(.rf_radio_full);
static rev_fsm_mge     g_full_packet    AT(.rf_radio_full);
static rfr_dec         g_full_dec       AT(.rf_radio_full);

static u32 full_dec_ibuff[1024 * 2 / 4] AT(.rf_radio_full);

static u32 rfr_full_packet_cmd_buff[60 / 4] AT(.rf_radio_full);
static cbuffer_t cbuf_full_ack          AT(.rf_radio_full);
static u32 rfr_full_ack_buff[32 / 4]    AT(.rf_radio_full);

#define D_SELF_QUEUE   g_full_rf_mge.listener_queue

void rfr_full_unpacket_cmd_pool_init(void)
{
    cbuf_init(&g_full_packet.cmd_cbuf, &rfr_full_packet_cmd_buff[0], sizeof(rfr_full_packet_cmd_buff));
    vble_smpl_recv_register(&g_full_packet, (int (*)(void *, u8 *, u16))unpack_data_deal);
}

static void fd_is_need2idle(void)
{
    if ((RRA_ENC_IDLE == g_full_rf_mge.enc_status) &&     \
        (RRA_DEC_IDLE ==  g_full_rf_mge.dec_status)) {
        g_rf_radio_standby_jif_cnt = maskrom_get_jiffies() + 10;
    }
}
static void rfr_full_send_queue_init(void)
{
    cbuf_init(&cbuf_full_ack, &rfr_full_ack_buff[0], sizeof(rfr_full_ack_buff));
    rf_queue_init(&g_full_rf_mge.listener_queue, (void *)&rf_radio_ops, &cbuf_full_ack);
}

void rf_connect_set_mode(u8 mode)
{
    g_rfr_full_mode = mode;
}
static void rfr_full_encode_start(void)
{
    audio2rf_encoder_start((enc_obj *)sp_full_enc_obj);
    g_full_rf_mge.enc_status = RRA_ENCODING;
}

static void rfr_full_encode_stop(IS_WAIT enc_wait)
{
    if (NULL != sp_full_enc_obj) {
        sp_full_enc_obj = audio2rf_encoder_stop(sp_full_enc_obj, enc_wait, NEED_WAIT);
        delay_10ms(2);
        rf_queue_uninit(&g_full_rf_mge.speaker_queue, NEED_WAIT);
    }
    g_full_rf_mge.enc_status = RRA_ENC_IDLE;  //空闲状态i
    fd_is_need2idle();
}


static dec_obj *rfr_full_decode_start(RF_RADIO_ENC_HEAD *p_enc_head)
{
    dec_obj *p_dec_obj = rfr_decode_start_phy(
                             p_enc_head,
                             &g_full_dec,
                             &full_dec_ibuff[0],
                             sizeof(full_dec_ibuff),
                             rf2audio_receiver_kick_decoder,
                             &g_full_packet);


    if (NULL != p_dec_obj) {
        g_full_rf_mge.dec_status = RRA_DECODING;
        log_info("now dac_sr %d\n", dac_sr_read());
    }
    return p_dec_obj;
}

static void rfr_full_decode_stop(void)
{
    rfr_decode_stop_phy(&g_full_dec, &g_full_packet);
    g_full_rf_mge.dec_status = RRA_DEC_IDLE;
    fd_is_need2idle();
}


static u32 fd_rrapp_receiving_loop(void)
{
    rev_fsm_mge *p_recv_ops = &g_full_packet;
    u8 rra_packet[16] ALIGNED(2);        //所有对讲机
    u32 len = packet_cmd_get(&p_recv_ops->cmd_cbuf, &rra_packet[0], sizeof(rra_packet));
    int connect_handle;
    if (0 == len) {
        return E_PACKET_NULL;
    }
    switch (rra_packet[1]) {
    case AUDIO2RF_START_PACKET:
        log_info("DEC_AUDIO2RF_START_PACKET");
        if (RRA_DECODING == g_full_rf_mge.dec_status) {
            rra_send_ack_cmd(&D_SELF_QUEUE, AUDIO2RF_START_PACKET, 1);
            break;
        }
        RF_RADIO_ENC_HEAD *p_enc_head = (RF_RADIO_ENC_HEAD *)&rra_packet[2];
        dec_obj *t_dec_obj = rfr_full_decode_start(p_enc_head);
        if (NULL != t_dec_obj) {
            vble_smpl_ioctl(VBLE_SMPL_GET_CONN_HANDLE, (int)&connect_handle);
            if (0 != connect_handle) {
                ll_vendor_rx_max(connect_handle, ACL_RTX_DATA_LEN); //减少蓝牙最大接收数据长度以提高蓝牙接收数据成功率
            }
            rra_send_ack_cmd(&D_SELF_QUEUE, AUDIO2RF_START_PACKET, 1);
        } else {
            rfr_full_decode_stop();
            /* rra_decode_stop(); */
            rra_send_ack_cmd(&D_SELF_QUEUE, AUDIO2RF_START_PACKET, 0);
        }
        break;
    case AUDIO2RF_STOP_PACKET:
        log_info("STOP_AUDIO2RF_START_PACKET");
        rfr_full_decode_stop();
        /* rra_decode_stop(); */
        break;
    case (AUDIO2RF_ACK | AUDIO2RF_START_PACKET):
        log_info("ACK_AUDIO2RF_START_PACKET_ACK");
        u8 *cmd_data = &rra_packet[2];
        if ((1 == cmd_data[0]) && (RRA_ENC_WAITING_START_ACK == g_full_rf_mge.enc_status)) {
            vble_smpl_ioctl(VBLE_SMPL_GET_CONN_HANDLE, (int)&connect_handle);
            if (0 != connect_handle) {
                ll_vendor_rx_max(connect_handle, ACL_RTX_DATA_LEN); //减少蓝牙最大接收数据长度以提高蓝牙接收数据成功率
            }
            rfr_full_encode_start();
            /* rra_encode_start(); */
        } else {
            rfr_full_encode_stop(NO_WAIT);
            audio_adc_off_api();
            return 0;
        }
        break;
    }
    return 0;
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
static u8 fd_rrapp_loop(int active_msg)
{
    int msg[2], err, ble_status;
    u32 event, retry, target_jiff;
    bool bt_connecting = 0;

    //audio_adc_init_api(RA_ENC_SR, AUDIO_ADC_MIC, 0);

    if (NO_MSG != active_msg) {
        msg[0] = active_msg;
        goto __fd_rrapp_msg_deal;
    }
    while (1) {

        if ((RRA_DEC_IDLE == g_full_rf_mge.dec_status) && (RRA_ENC_IDLE == g_full_rf_mge.enc_status)) {
            vble_smpl_ioctl(VBLE_SMPL_GET_STATUS, (int)&ble_status);
            if ((ble_status == BLE_ST_NOTIFY_IDICATE)/*从机已连接并可发数*/ || \
                (ble_status == BLE_ST_SEARCH_COMPLETE)/*主机已连接并可发数*/) {
                bt_connecting = 1;
                if (time_after(maskrom_get_jiffies(), g_rf_radio_standby_jif_cnt)) {
                    g_rfr_full_mode = RFR_FULL_IDLE;
                    goto __exit_fd_rrapp_loop;
                }
            } else {
                //蓝牙断开连接  或  未曾连接过
                if (1 == bt_connecting) {
                    bt_connecting = 0;
                    app_softoff_time_reset(g_rf_radio_softoff_jif);
                }
#if APP_SOFTOFF_CNT_TIME_10MS
                if (time_after(maskrom_get_jiffies(), g_rf_radio_softoff_jif)) {
                    post_msg(1, MSG_POWER_OFF);
                }
#endif
            }
        }
        err = get_msg(2, &msg[0]);
        if (MSG_NO_ERROR != err) {
            msg[0] = NO_MSG;
            log_info("get msg err 0x%x\n", err);
        }

        if (RRA_ENC_WAITING_START_ACK == g_full_rf_mge.enc_status) {
            if (time_after(maskrom_get_jiffies(), target_jiff)) {
                log_info("resending start packet!\n");
                audio2rf_start_cmd(&g_full_rf_mge.speaker_queue, sp_full_enc_obj->info.sr, sp_full_enc_obj->info.br, RA_ENC_TYPE);
                target_jiff = maskrom_get_jiffies() + 10;
                if (0 == retry--) {
                    log_info("radio_start fail!\n");
                    rfr_full_encode_stop(NO_WAIT);
                    audio_adc_off_api();
                }
            }
        }

        fd_rrapp_receiving_loop();


__fd_rrapp_msg_deal:
        switch (msg[0]) {
        case MSG_SENDER_START:
            if ((RRA_ENCODING == g_full_rf_mge.enc_status) || \
                (RRA_ENC_WAITING_START_ACK == g_full_rf_mge.enc_status)) {
                break;
            }
            vble_smpl_ioctl(VBLE_SMPL_GET_STATUS, (int)&ble_status);
            if ((ble_status == BLE_ST_NOTIFY_IDICATE)/*从机已连接并可发数*/ || \
                (ble_status == BLE_ST_SEARCH_COMPLETE)/*主机已连接并可发数*/) {
                if (RRA_DEC_IDLE == g_full_rf_mge.dec_status) {
                    vble_smpl_ioctl(VBLE_SMPL_UPDATE_CONN_INTERVAL, RADIO_WORKING_INTERVAL);
                }

                audio_adc_init_api(RA_ENC_SR, AUDIO_ADC_MIC, 0);
                sp_full_enc_obj = audio2rf_encoder_io(\
                                                      RA_ENC_FUNC,                        \
                                                      (void *)&g_full_rf_mge.speaker_queue, \
                                                      (void *)&rf_radio_ops);

                if (NULL != sp_full_enc_obj) {
                    audio2rf_start_cmd(&g_full_rf_mge.speaker_queue, sp_full_enc_obj->info.sr, sp_full_enc_obj->info.br, RA_ENC_TYPE);
                    g_full_rf_mge.enc_status = RRA_ENC_WAITING_START_ACK;
                    target_jiff = maskrom_get_jiffies() + 5;
                    retry = 5;
                } else {
                    g_full_rf_mge.enc_status = RRA_ENC_IDLE;
                }
            } else {
                log_info("Radio isn't Ready! status:%d %d\n", g_full_rf_mge.dec_status, ble_status);
            }
            break;
        case MSG_SENDER_STOP:
            log_info("MSG_SENDER_STOP\n");
            rfr_full_encode_stop(NEED_WAIT);
            audio_adc_off_api();
            break;
        case MSG_WFILE_FULL:
        case MSG_SENDER_STOP_NOW:
            log_info("MSG_SENDER_STOP_NOW\n");
            rfr_full_encode_stop(NO_WAIT);
            audio_adc_off_api();
            break;
        case MSG_BLE_ROLE_SWITCH:
            log_info("MSG_BLE_ROLE_SWITCH\n");
            if ((RRA_DEC_IDLE != g_full_rf_mge.dec_status) ||    \
                (RRA_ENC_IDLE != g_full_rf_mge.enc_status)) {
                break;
            }
            vble_smpl_recv_register(NULL, NULL);
            vble_smpl_exit();
            vble_smpl_switch_role(); //主从内部切换
            vble_smpl_init();
            vble_smpl_recv_register(&g_full_packet, (int (*)(void *, u8 *, u16))unpack_data_deal);
            break;
        case MSG_CHANGE_WORK_MODE:
            log_info("MSG_CHANGE_WORK_MODE\n");
            g_rfr_full_mode = RFR_FULL_EXIT;
            goto __exit_fd_rrapp_loop;
        case MSG_500MS:
            wdt_clear();
            vble_smpl_ioctl(VBLE_SMPL_GET_STATUS, (int)&ble_status);
            if ((ble_status != BLE_ST_NOTIFY_IDICATE)/*从机断开*/ && \
                (ble_status != BLE_ST_SEARCH_COMPLETE)/*主机断开*/) {
                //蓝牙断开连接  或  未曾连接过
                if (RRA_DECODING == g_full_rf_mge.dec_status) {
                    rfr_full_decode_stop();
                }
                if (RRA_ENCODING == g_full_rf_mge.enc_status) {
                    log_info("disconn stop enc\n");
                    rfr_full_encode_stop(NO_WAIT);
                    audio_adc_off_api();
                }
            }

        default:
            ap_handle_hotkey(msg[0]);
            break;
        }
    }

__exit_fd_rrapp_loop:


    if (RRA_DECODING == g_full_rf_mge.dec_status) {
        rfr_full_decode_stop();
    }
    if (RRA_ENCODING == g_full_rf_mge.enc_status) {
        rfr_full_encode_stop(NO_WAIT);
        audio_adc_off_api();
    }
    return g_rfr_full_mode;
}

/*----------------------------------------------------------------------------*/
/**@brief   全双工对讲机应用主函数
   @param
   @return
   @author
   @note    主函数根据不同状态运行不同子函数；
            蓝牙已连接且应用空闲时，执行rrapp_idle函数低功耗保持连接
*/
/*----------------------------------------------------------------------------*/
void rf_radio_full_duplex_loop(void)
{

    int msg = NO_MSG;
    memset(&g_full_rf_mge,  0, sizeof(g_full_rf_mge));
    memset(&g_full_dec,    0, sizeof(g_full_dec));
    memset(&g_full_packet, 0, sizeof(g_full_packet));

    g_full_rf_mge.enc_status = RRA_ENC_IDLE;
    g_full_rf_mge.dec_status = RRA_DEC_IDLE;
    rra_timeout_reset();

    rfr_full_send_queue_init();         //ACK队列注册
    rfr_full_unpacket_cmd_pool_init();  //CMD命令cbuff注册给拆包

    g_rfr_full_mode = RFR_FULL_DUPLEX;

    while (1) {

        switch (g_rfr_full_mode) {
        case RFR_FULL_IDLE:
            /* 应用休眠闲状态 */
            msg = rrapp_idle(&g_full_packet);
            break;
        case RFR_FULL_DUPLEX:
            /* 全双工对讲状态 */
            g_rfr_full_mode = fd_rrapp_loop(msg);
            if (RFR_FULL_EXIT == g_rfr_full_mode) {
                goto __rf_radio_full_duplex_app_exit;
            }
            break;
        default:
            break;
        }

    }
__rf_radio_full_duplex_app_exit:
    return;
}

#endif

