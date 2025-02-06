#include "hot_msg.h"
#include "vble_simple.h"
#include "msg.h"
#include "trans_unpacket.h"
#include "tick_timer_driver.h"
#include "audio_dac.h"
#include "rf2audio_recv.h"
#include "hci_ll.h"

#include "rf_radio_app.h"
#include "rf_radio_comm.h"
#include "connect_radio.h"
#include "rf_half_duplex.h"

#include "app_modules.h"
#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[rf_half_rece]"
#include "log.h"

#if (RF_RADIO_EN & BLE_EN & TESTE_BLE_EN)

//------
extern connect_rf_mge g_half_rf_mge;
extern rev_fsm_mge g_half_packet;   //拆包命令池 / 解码句柄
rfr_dec g_half_dec AT(.rf_radio_half);      //rece 和 conn的初始化

static u32 rfr_half_packet_cmd_buff[60 / 4] AT(.rf_radio_half);
static u32 half_dec_ibuff[1024 * 2 / 4]     AT(.rf_radio_half);
//---------

void rfr_half_unpacket_cmd_pool_init(void)
{
    cbuf_init(&g_half_packet.cmd_cbuf, &rfr_half_packet_cmd_buff[0], sizeof(rfr_half_packet_cmd_buff));
    vble_smpl_recv_register(&g_half_packet, (int (*)(void *, u8 *, u16))unpack_data_deal);
}

dec_obj *rfr_half_decode_start(RF_RADIO_ENC_HEAD *p_enc_head)
{

    dec_obj *p_dec_obj = rfr_decode_start_phy(
                             p_enc_head,
                             &g_half_dec,
                             &half_dec_ibuff[0],
                             sizeof(half_dec_ibuff),
                             rf2audio_receiver_kick_decoder,
                             &g_half_packet);
    if (NULL != p_dec_obj) {
        g_half_rf_mge.dec_status = RRA_DECODING;
        log_info("now dac_sr %d\n", dac_sr_read());
    }
    return p_dec_obj;
}

void rfr_half_decode_stop(void)
{
    log_info("rra_dec_stop\n");

    rfr_decode_stop_phy(&g_half_dec, &g_half_packet);
    g_half_rf_mge.dec_status = RRA_DEC_IDLE;
    g_rf_radio_standby_jif_cnt = maskrom_get_jiffies() + 10;
    /* rra_timeout_reset(); */
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
    rev_fsm_mge *p_recv_ops = &g_half_packet;
    int connect_handle;
    u8 rra_packet[16] ALIGNED(2);        //所有对讲机
    u32 len = packet_cmd_get(&p_recv_ops->cmd_cbuf, &rra_packet[0], sizeof(rra_packet));
    if (0 == len) {
        return E_PACKET_NULL;
    }
    RF_RADIO_ENC_HEAD *p_enc_head;
    switch (rra_packet[1]) {
    case AUDIO2RF_START_PACKET:
        p_enc_head = (RF_RADIO_ENC_HEAD *)&rra_packet[2];
        log_info("*******AUDIO2RF_START_PACKET********\n");
        if (RRA_DECODING == g_half_rf_mge.dec_status) {
            rra_send_ack_cmd(&g_half_rf_mge.listener_queue, AUDIO2RF_START_PACKET, 2);
            break;
        }

        dec_obj *t_dec_obj = rfr_half_decode_start(p_enc_head);
        if (NULL != t_dec_obj) {
            /* rra_decode_start(); */
            rra_send_ack_cmd(&g_half_rf_mge.listener_queue, AUDIO2RF_START_PACKET, 1);
            vble_smpl_ioctl(VBLE_SMPL_GET_CONN_HANDLE, (int)&connect_handle);
            if (0 != connect_handle) {
                ll_vendor_rx_max(connect_handle, ACL_RTX_DATA_LEN); //减少蓝牙最大接收数据长度以提高蓝牙接收数据成功率
            }
        } else {
            rfr_half_decode_stop();
            rra_send_ack_cmd(&g_half_rf_mge.listener_queue, AUDIO2RF_START_PACKET, 0);
        }
        break;
    case AUDIO2RF_STOP_PACKET:
        log_info("*******AUDIO2RF_STOP_PACKET*******, %d\n", rra_packet[2]);
        rfr_half_decode_stop();
        break;
    }
    return 0;
}

void rrapp_receiving(int active_msg)
{
    int msg[2], err, ble_status;
    u32 event;
    u32 res;
    bool bt_flag = 0;

    rra_timeout_reset();
    if (NO_MSG != active_msg) {
        msg[0] = active_msg;
        active_msg = NO_MSG;
        goto __rrapp_msg_deal;
    }
    /* log_info("rrapp_receiving\n"); */
    while (1) {
        if (RRA_DEC_IDLE == g_half_rf_mge.dec_status) {
            vble_smpl_ioctl(VBLE_SMPL_GET_STATUS, (int)&ble_status);
            if ((ble_status == BLE_ST_NOTIFY_IDICATE)/*从机已连接并可发数*/ || \
                (ble_status == BLE_ST_SEARCH_COMPLETE)/*主机已连接并可发数*/) {
                bt_flag = 1;
                if (time_after(maskrom_get_jiffies(), g_rf_radio_standby_jif_cnt)) {
                    g_rfr_half_mode = RFR_HALF_IDLE_MODE;
                    goto __exit_rrapp_receiving;
                }
            } else {
                if (1 == bt_flag) {
                    bt_flag = 0;
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
        res = radio_rf_receiving_loop();
__rrapp_msg_deal:
        switch (msg[0]) {
        case MSG_SENDER_START:
            if (RRA_DEC_IDLE == g_half_rf_mge.dec_status) {
                vble_smpl_ioctl(VBLE_SMPL_GET_STATUS, (int)&ble_status);
                if ((ble_status == BLE_ST_NOTIFY_IDICATE)/*从机已连接并可发数*/ || \
                    (ble_status == BLE_ST_SEARCH_COMPLETE)/*主机已连接并可发数*/) {
                    g_rfr_half_mode = RFR_HALF_SEND_MODE;
                    log_info("receiver MSG_SENDER_START\n");
                    goto __exit_rrapp_receiving;
                } else {
                    log_info("Radio isn't Ready! status:%d %d\n", g_half_rf_mge.dec_status, ble_status);
                }
            }
            break;
        case MSG_BLE_ROLE_SWITCH:
            log_info("MSG_BLE_ROLE_SWITCH\n");
            vble_smpl_update_parm_succ_register(NULL);
            vble_smpl_recv_register(NULL, NULL);
            vble_smpl_exit();
            vble_smpl_switch_role(); //主从内部切换
            vble_smpl_init();
            vble_smpl_recv_register(&g_half_packet, (int (*)(void *, u8 *, u16))unpack_data_deal);
            rfr_half_bt2applyinit();                //蓝牙更新连接间隔注册
            break;
        case MSG_CHANGE_WORK_MODE:
            log_info("MSG_CHANGE_WORK_MODE\n");
            g_rfr_half_mode = RFR_HALF_EXIT;
            goto __exit_rrapp_receiving;
        case MSG_500MS:
            wdt_clear();
            if (RRA_DECODING == g_half_rf_mge.dec_status) {
                vble_smpl_ioctl(VBLE_SMPL_GET_STATUS, (int)&ble_status);
                if ((ble_status != BLE_ST_NOTIFY_IDICATE)/*从机断开*/ && \
                    (ble_status != BLE_ST_SEARCH_COMPLETE)/*主机断开*/) {
                    rfr_half_decode_stop();
                }
            }
        default:
            ap_handle_hotkey(msg[0]);
            break;
        }
    }
__exit_rrapp_receiving:
    if (NULL != g_half_dec.obj) {
        rfr_half_decode_stop();
    }
}

#endif
