#include "rf_half_duplex.h"
#include "connect_radio/connect_radio.h"
#include "rf_radio_comm.h"

#include "trans_unpacket.h"
#include "rf_send_queue.h"

#include "bt_ble.h"
#include "vble_simple.h"

#include "msg.h"
#include "circular_buf.h"
#include "app_modules.h"

#if defined (FULL_DUPLEX_RADIO) && (!FULL_DUPLEX_RADIO)

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[rf_half]"
#include "log.h"

enum RFR_HALF_MODE g_rfr_half_mode;
static apply_bt_info_struct apply_bt_info;

connect_rf_mge g_half_rf_mge AT(.rf_radio_half);
rev_fsm_mge    g_half_packet AT(.rf_radio_half);
extern rfr_dec g_half_dec;

static cbuffer_t cbuf_half_ack          AT(.rf_radio_half);
static u32 rfr_half_ack_buff[32 / 4]    AT(.rf_radio_half);


void rfr_half_send_queue_init(void)
{
    /* ACK队列注册 */
    cbuf_init(&cbuf_half_ack, &rfr_half_ack_buff[0], sizeof(rfr_half_ack_buff));
    rf_queue_init(&g_half_rf_mge.listener_queue, (void *)&rf_radio_ops, &cbuf_half_ack);
}

void bt_update2apply(int update_interval)
{
    /* 蓝牙更新参数成功后返回给应用层 */
    apply_bt_info.bt_interval = update_interval;
}

u32 get_curr_true_interval(void)
{
    int res = apply_bt_info.bt_interval;
    apply_bt_info.bt_interval = 0;
    return res;
}

void rfr_half_bt2applyinit(void)
{
    vble_smpl_update_parm_succ_register((void (*)(int))bt_update2apply);
}
void rf_connect_set_mode(u8 mode)
{
    g_rfr_half_mode = mode;
}
/*----------------------------------------------------------------------------*/
/**@brief   半双工对讲机应用主函数
   @param
   @return
   @author
   @note    主函数根据不同状态运行不同子函数；
            蓝牙已连接且应用空闲时，执行rrapp_idle函数低功耗保持连接
*/
/*----------------------------------------------------------------------------*/
void rf_radio_half_duplex_loop(void)
{

    int msg = NO_MSG;
    memset(&g_half_rf_mge, 0, sizeof(g_half_rf_mge));
    memset(&g_half_packet, 0, sizeof(g_half_packet));
    memset(&g_half_dec, 0, sizeof(g_half_dec));

    g_half_rf_mge.enc_status = RRA_ENC_IDLE;
    g_half_rf_mge.dec_status = RRA_DEC_IDLE;
    rra_timeout_reset();                    //reset soft_off时间

    rfr_half_bt2applyinit();                //蓝牙更新连接间隔注册
    rfr_half_send_queue_init();             //ACK队列注册
    rfr_half_unpacket_cmd_pool_init();      //CMD命令cbuff注册给拆包

    g_rfr_half_mode = RFR_HALF_RECIVE_MODE;      //所有消息都先去到这个分支处理
    while (1) {
        switch (g_rfr_half_mode) {
        case RFR_HALF_IDLE_MODE:
            /* 应用休眠闲状态 */
            msg = rrapp_idle(&g_half_packet);
            g_rfr_half_mode = RFR_HALF_RECIVE_MODE; //所有消息都先去到这个分支处理
            break;
        case RFR_HALF_SEND_MODE:
            /* 编码发送状态 */
            rrapp_sending(msg);
            msg = NO_MSG;
            break;
        case RFR_HALF_RECIVE_MODE:
            /* 解码接收状态 */
            rrapp_receiving(msg);
            msg = NO_MSG;
            if (RFR_HALF_EXIT == g_rfr_half_mode) {
                goto __rf_radio_half_duplex_app_exit;
            }
            break;
        default:
            break;

        }
    }

__rf_radio_half_duplex_app_exit:
    return;
}
#endif
