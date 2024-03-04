#include "rf_radio_app.h"
#include "rf_radio_rs.h"
#include "trans_packet.h"
#include "key.h"
#include "audio.h"
#include "power_api.h"
#include "rf_pa_port.h"

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[rf_radio]"
#include "log.h"

#if (RF_RADIO_EN & BLE_EN & TESTE_BLE_EN)
#if APP_SOFTOFF_CNT_TIME_10MS
#define app_softoff_time_reset(n) (n = maskrom_get_jiffies() + APP_SOFTOFF_CNT_TIME_10MS)
#else
#define app_softoff_time_reset(n)
#endif

#define RADIO_DEFAULT_ROLE_MASTER   0

radio_mge_struct radio_mge AT(.rf_radio_data);
static u32 rra_packet_cmd_buff[60 / 4] AT(.rf_radio_data);
u8 rra_packet[16] ALIGNED(2) AT(.rf_radio_data);
static u32 radio_ack_buff[32 / 4] AT(.rf_radio_data);

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

void rra_in_idle(void)
{
    radio_mge.rra_status = RRA_IDLE;
    app_softoff_time_reset(radio_mge.app_softoff_jif_cnt);
    radio_mge.app_standby_jif_cnt = maskrom_get_jiffies() + 10;
}


/*----------------------------------------------------------------------------*/
/**@brief   void rrapp_send_queue_init(void)
   @param   无
   @return  无
   @author  liujie
   @note    独立的消息队列的初始化
*/
/*----------------------------------------------------------------------------*/
void rrapp_send_queue_init(void)
{
    cbuf_init(&radio_mge.ack_cbuf, &radio_ack_buff[0], sizeof(radio_ack_buff));
    rf_send_soft_isr_init(&radio_mge.ack_cbuf);
}

u32 rra_send_ack_cmd(u8 ack_cmd, u8 ack_data)
{
    u32 res = audio2rf_ack_cmd(ack_cmd, &ack_data, sizeof(ack_data));
    if (0 != res) {
        log_error("ack can't push in queue, 0x%x!!!\n");
    }
    return res;
}


static void rf_radio_app_init(void)
{
    sysmem_write_api(SYSMEM_INDEX_SYSMODE, &work_mode, sizeof(work_mode));
    /* overlay变量初始化 */
    audio_rf_clr_buf();
    memset(&radio_mge, 0, sizeof(radio_mge));
    cbuf_init(&radio_mge.cmd_cbuf, &rra_packet_cmd_buff[0], sizeof(rra_packet_cmd_buff));
    radio_mge.packet_recv.cmd_pool = &radio_mge.cmd_cbuf;
    rrapp_send_queue_init();
    /* cbuf_init(&radio_mge.ack_cbuf, &radio_ack_buff[0], sizeof(radio_ack_buff)); */
    /* rf_send_soft_isr_init(&radio_mge.ack_cbuf); */
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
#if RF_PA_EN
    rf_pa_io_sel();
#endif
#if RADIO_DEFAULT_ROLE_MASTER
    vble_smpl_master_select();
#else
    vble_smpl_slave_select();
#endif
    vble_smpl_init();
    vble_smpl_recv_register(&radio_mge.packet_recv, unpack_data_deal);
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
    if (RRA_ENCODING == radio_mge.rra_status) {
        rra_encode_stop(ENC_NO_WAIT);
    } else if (RRA_DECODING == radio_mge.rra_status) {
        rra_decode_stop();
    }
    audio_init();
    dac_init_api(SR_DEFAULT);
    audio2rf_send_register(NULL);
    /* #if TRANS_DATA_HID_EN */
    /*     vble_slave_recv_cb_register(ATT_SLV2MSTR_RF_RADIO_IDX, NULL); */
    /* #elif TRANS_DATA_SPPLE_EN */
    /*     vble_master_recv_cb_register(ATT_SLV2MSTR_RF_RADIO_IDX, NULL); */
    /* #elif TESTE_BLE_EN */
    vble_smpl_recv_register(NULL, NULL);
    vble_smpl_exit();
    /* #endif */
    key_table_sel(NULL);
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
    vble_smpl_ioctl(VBLE_SMPL_UPDATE_CONN_INTERVAL, RADIO_STANDBY_INTERVAL);
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
        rev_fsm_mge *p_recv_ops = &radio_mge.packet_recv;
        if (cbuf_get_data_size(p_recv_ops->cmd_pool)) {
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
            当外部事件触发时退出standby，运行recevint和sending函数响应消息、事件
* /
/*----------------------------------------------------------------------------*/
void rf_radio_app(void)
{
    int msg = NO_MSG;

    u32 flag;

    rf_radio_app_init();

    rra_in_idle();

    rrapp_open();

    while (1) {

        switch (radio_mge.rra_status) {
        case RRA_STANDBY:
            /* 应用休眠闲状态 */
            msg = rrapp_standby();
            break;
        case RRA_GOTO_ENC:
            /* 编码发送状态 */
            rrapp_sending(msg);
            break;
        case RRA_IDLE:
            /* 解码接收状态 */
            flag = rrapp_receiving(msg);
            if (!flag) {
                goto __rf_radio_app_exit;
            }
            break;
        }

    }
__rf_radio_app_exit:
    rf_radio_app_uninit();
    return;
}
#endif
