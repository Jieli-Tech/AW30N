#include "rf_half_duplex.h"
#include "rf_full_duplex.h"
#include "connect_radio.h"

#include "trans_unpacket.h"
#include "audio2rf_send.h"
#include "power_api.h"
#include "vble_simple.h"
#include "msg.h"
#include "wdt.h"
#include "tick_timer_driver.h"
#include "hci_ll.h"
#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[rf_radio]"
#include "log.h"

#if (RF_RADIO_EN & BLE_EN & TESTE_BLE_EN)

#define RADIO_DEFAULT_ROLE_MASTER   0

extern u8 g_rf_radio_occur;     //ACL对讲机(虽然广播不需要用到)
/*----------------------------------------------------------------------------*/
/**@brief   对讲机应用无线传输发送
   @param   data:发送的数据buff
            len :发送的数据长度
   @return  0:成功   非0:失败，错误值请查看errno-base.h
   @author
   @note
*/
/*----------------------------------------------------------------------------*/
/* ACL对讲机使用 */
static int rf_radio_send_api(u8 *data, u16 len)
{
    int ret = 0;
    ret = vble_smpl_send_api(data, len);
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
/* ACL对讲机使用 */
static int rf_radio_check_status_api(void)
{
    int ble_status;
    vble_smpl_ioctl(VBLE_SMPL_GET_STATUS, (int)&ble_status);
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
/* ACL对讲机使用 */
static int rf_radio_get_valid_len_api(void)
{
    return get_buffer_vaild_len(0);
}

/* ACL对讲机使用 */
const audio2rf_send_mge_ops rf_radio_ops = {
    .send = rf_radio_send_api,
    .check_status = rf_radio_check_status_api,
    .get_valid_len = rf_radio_get_valid_len_api,
};

/* ACL对讲机使用 */
u32 rra_send_ack_cmd(queue_obj *p_queue, u8 ack_cmd, u8 ack_data)
{
    u32 res = audio2rf_ack_cmd(p_queue, ack_cmd, &ack_data, sizeof(ack_data));
    if (0 != res) {
        log_error("acl_ack can't push in queue, 0x%x!!!\n", res);
    }
    return res;
}

/* ACL对讲机使用 */
static void connect_radio_bt_init()
{
    ble_clock_init();

#if RF_PA_EN
    rf_pa_io_sel();
#endif

#if RADIO_DEFAULT_ROLE_MASTER
    vble_smpl_master_select();
#else
    vble_smpl_slave_select();
#endif
    vble_smpl_init();
}


/* ACL对讲机使用 */
static void connect_radio_bt_uninit(void)
{
#if RF_PA_EN
    rf_pa_io_close();
#endif
    vble_smpl_recv_register(NULL, NULL);
    vble_smpl_exit();
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
/* ACL对讲机使用 */
static int rrapp_exit_standby(int msg, u8 mode)
{
    rf_connect_set_mode(mode);
    rra_timeout_reset();
    audio_init();
    return msg;
}

/* ACL对讲机使用 */
int rrapp_idle(rev_fsm_mge *p_recv_ops)
{
    log_info("---idle--");
    rra_timeout_reset();
    audio_off();
    g_rf_radio_occur = 0;
#if FULL_DUPLEX_RADIO
    u8 tx_mode = RFR_FULL_DUPLEX; //所有消息都先去到这个分支处理
    u8 rx_mode = RFR_FULL_DUPLEX; //所有消息都先去到这个分支处理
#else
    u8 tx_mode = RFR_HALF_SEND_MODE;
    u8 rx_mode = RFR_HALF_RECIVE_MODE;
#endif

    bool has_event = 0;
    int ble_status, err, msg[2];
    msg[0] = NO_MSG;
#if 1
    log_info("dec and enc idle ,goto 50ms\n");
    vble_smpl_ioctl(VBLE_SMPL_UPDATE_CONN_INTERVAL, RADIO_STANDBY_INTERVAL);    //退出模式时更新回standby
    int connect_handle;
    vble_smpl_ioctl(VBLE_SMPL_GET_CONN_HANDLE, (int)&connect_handle);
    if (0 != connect_handle) {
        ll_vendor_rx_max(connect_handle, 255); //恢复蓝牙最大接收数据长度
    }
#endif
    while (1) {
        sys_power_down(2000000);
        wdt_clear();
#if APP_SOFTOFF_CNT_TIME_10MS
        /* 判断定时关机 */
        if (time_after(maskrom_get_jiffies(), g_rf_radio_softoff_jif)) {
            return rrapp_exit_standby(MSG_POWER_OFF, rx_mode);
        }
#endif

        vble_smpl_ioctl(VBLE_SMPL_GET_STATUS, (int)&ble_status);
        if ((ble_status != BLE_ST_NOTIFY_IDICATE)/*从机已连接并可发数*/ && \
            (ble_status != BLE_ST_SEARCH_COMPLETE)/*主机已连接并可发数*/) {
            /* 断开连接，退出idle回到活跃状态等待下一次连接 */
            log_info("ble conn status :%d\n", ble_status);
            return rrapp_exit_standby(NO_MSG, rx_mode);
        }

        has_event = has_sys_event(); //系统事件发生，退出idle
        if (has_event) {
            log_info("has sys_event\n");
            return rrapp_exit_standby(NO_MSG, rx_mode);
        }
        if (g_rf_radio_occur) { //应用事件发生，退出idle
            log_info("rf_radio event occur\n");
            g_rf_radio_occur = 0;
            return rrapp_exit_standby(NO_MSG, rx_mode);
        }
        /* rev_fsm_mge *p_recv_ops = &radio_mge.packet_recv; */
        if (cbuf_get_data_size(&p_recv_ops->cmd_cbuf)) {
            return rrapp_exit_standby(NO_MSG, rx_mode);
        }

        err = get_msg_phy(2, &msg[0], 0);//获取到指定消息，退出idle
        switch (msg[0]) {
        /* 需要响应的消息 */
        /* case MSG_POWER_OFF: */
        /*     log_info("has msg:0x%x\n", msg[0]); */
        /*     return rrapp_exit_standby(msg[0]); */

        case MSG_SENDER_START:
        case MSG_SENDER_STOP:
            return rrapp_exit_standby(msg[0], tx_mode);
        case MSG_BLE_ROLE_SWITCH:
        case MSG_CHANGE_WORK_MODE:
            goto __rrapp_idle_exit_;
        case MSG_500MS:
        default:
            msg[0] = NO_MSG;
            break;
        }
    }
__rrapp_idle_exit_:
    return rrapp_exit_standby(msg[0], rx_mode);
}


void rf_radio_connect_app(void)
{

    connect_radio_bt_init();    //蓝牙初始化
    audio_init();

#if defined (FULL_DUPLEX_RADIO) && (FULL_DUPLEX_RADIO)
    /* 全双工对讲机 */
    rf_radio_full_duplex_loop();
#else
    /* 半双工对讲机 */
    rf_radio_half_duplex_loop();
#endif

    connect_radio_bt_uninit();
}




#endif
