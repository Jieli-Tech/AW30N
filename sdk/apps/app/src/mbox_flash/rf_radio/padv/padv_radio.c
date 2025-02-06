
#include "trans_packet.h"
#include "trans_unpacket.h"
#include "key.h"
#include "audio.h"
#include "power_api.h"
#include "rf_pa_port.h"
#include "hot_msg.h"
#include "audio_adc_api.h"
#include "audio_dac_api.h"
#include "msg.h"
#include "clock.h"
#include "vble_adv.h"

#include "rf2audio_recv.h"
#include "audio2rf_send.h"
#include "decoder_mge.h"

#include "rf_radio_app.h"
#include "rf_radio_comm.h"
#include "padv_radio.h"
#if defined (PADVB_WL_MODE) && (PADVB_WL_MODE)
#include "third_party/padvb_intercom/padvb.h"
#include "encoder_stream.h"

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[padv_radio]"
#include "log.h"


#define MSG_SET_RX_STATUS MSG_BLE_ROLE_SWITCH //节省资源，直接使用传统对讲机的按键分布

static enc_obj *sp_padv_enc_obj;
static RF_RADIO_ENC_HEAD s_enc_head;

static padv_mge     g_padv_radio AT(.rf_radio_padv);
static rfr_dec      g_padv_dec   AT(.rf_radio_padv);
static rev_fsm_mge  padv_packet  AT(.rf_radio_padv);

static u32 padv_dec_ibuff[1024 * 2 / 4] AT(.rf_radio_padv);
static u32 padv_packet_cmd_buff[60 / 4] AT(.rf_radio_padv);


static queue_obj *padv_get_send_queue_obj(void)
{
    queue_obj *obj = &g_padv_radio.speaker_queue;
    return obj;
}
const queue2vble_tx_send_mge_ops padv_ops = {
    .get_send_queue_obj = padv_get_send_queue_obj,
    .read_data = read_data_from_queue,
};


static void rf_radio_padv_init(void)
{
    ble_clock_init();
#if RF_PA_EN
    rf_pa_io_open();
#endif
    vble_adv_init();
}

static void rf_radio_padv_uninit(void)
{
#if RF_PA_EN
    rf_pa_io_close();
#endif
    vble_adv_uninit();
}

void padv_radio_post_stop_cmd()
{
    packet_cmd_post(&padv_packet.cmd_cbuf, AUDIO2RF_STOP_PACKET, NULL, 0);
}

void padv_unpacket_cmd_pool_init(void)
{
    cbuf_init(&padv_packet.cmd_cbuf, &padv_packet_cmd_buff[0], sizeof(padv_packet_cmd_buff));
    vble_adv_rx_cb_register(\
                            &padv_packet,                                   \
                            (void (*)(void))padv_radio_post_stop_cmd,       \
                            (int (*)(void *, u8 *, u16))unpack_data_deal);

}

static void padv_app_init(void)
{
    memset(&s_enc_head, 0, sizeof(s_enc_head));
    g_padv_radio.padv_tx_status = PADV_TX_IDLE;
    g_padv_radio.padv_rx_status = PADV_RX_IDLE;
}

dec_obj *padv_decode_start(RF_RADIO_ENC_HEAD *p_enc_head)
{
    dec_obj *p_dec_obj = rfr_decode_start_phy(
                             p_enc_head,
                             &g_padv_dec,
                             &padv_dec_ibuff[0],
                             sizeof(padv_dec_ibuff),
                             rf2audio_receiver_kick_decoder,
                             &padv_packet);
    return p_dec_obj;
}

void padv_decode_stop(void)
{
    rfr_decode_stop_phy(&g_padv_dec, &padv_packet);
    memset(&s_enc_head, 0, sizeof(s_enc_head));
}



/*----------------------------------------------------------------------------*/
/**@brief   广播rx接收状态处理
   @param
   @return
   @author
   @note    接收状态下响应接收事件
**/
/*----------------------------------------------------------------------------*/
u32 radio_padv_receiving_loop(void)
{
    rev_fsm_mge *p_recv_ops = &padv_packet;
    u8 rra_packet[16] ALIGNED(2);
    u32 len = packet_cmd_get(&p_recv_ops->cmd_cbuf, &rra_packet[0], sizeof(rra_packet));
    if (0 == len) {
        return E_PACKET_NULL;
    }
    switch (rra_packet[1]) {
    case AUDIO2RF_START_PACKET:
        log_info("AUDIO2RF_START_PACKET\n");
        if (PADV_TX_IDLE != g_padv_radio.padv_tx_status) {
            log_info("encoding\n");
            break;
        }
        if (PADV_RX_IDLE == g_padv_radio.padv_rx_status) {
            log_info("rx_no_ready, cannot_decode\n");
            break;
        }
        RF_RADIO_ENC_HEAD *p_enc_head = (RF_RADIO_ENC_HEAD *)&rra_packet[2];
        if (NULL != g_padv_dec.obj) {
            u32 res = memcmp(p_enc_head, &s_enc_head, sizeof(s_enc_head));//防止没有stop包
            if (0 == res) {
                break;
            }
            padv_decode_stop();

        }
        memcpy(&s_enc_head, p_enc_head, sizeof(s_enc_head));
        padv_decode_start(p_enc_head);
        break;
    case AUDIO2RF_STOP_PACKET:
        log_info("AUDIO2RF_STOP_PACKET\n");
        padv_decode_stop();
        break;
    }
    return 0;
}
void padv_exit_rx(void)
{
    if (PADV_RX_IDLE != g_padv_radio.padv_rx_status) {
        vble_adv_rx_close();
        vble_adv_rx_uninit();
    }

    padv_decode_stop();
    g_padv_radio.padv_rx_status = PADV_RX_IDLE;
}

/* #include "encoder_mge.h" */
void padv_exit_tx(IS_WAIT enc_wait)
{
    if (NULL != sp_padv_enc_obj) {
        audio2rf_encoder_stop(sp_padv_enc_obj, enc_wait, NEED_WAIT);
        delay_10ms(20);
        sp_padv_enc_obj = NULL;
    }
    audio_adc_off_api();

    vble_adv_tx_close(); //为了消耗完编码数据,tx放最后关
    vble_adv_tx_uninit();

    g_padv_radio.padv_tx_status = PADV_TX_IDLE;
}




/*----------------------------------------------------------------------------*/
/**@brief   对讲机应用主函数
   @param
   @return
   @author
   @note    主函数根据不同状态运行不同子函数；
            蓝牙已连接且应用空闲时，执行standby函数低功耗保持连接
            当外部事件触发时退出standby，运行recevint和sending函数响应消息、事件
*/
/*----------------------------------------------------------------------------*/
void rf_radio_padv_loop(void)
{
    vble_padv_param_init();
    queue2vble_tx_send_register((void *)&padv_ops);
    //----
    padv_unpacket_cmd_pool_init();

    int msg[2];
    u32 err, ret, res;
    while (1) {
        err = get_msg(2, &msg[0]);
        if (MSG_NO_ERROR != err) {
            msg[0] = NO_MSG;
            log_info("get msg err 0x%x\n", err);
        }
        res = radio_padv_receiving_loop();
        switch (msg[0]) {
        case MSG_SENDER_START:
            /* log_info("SPEAKER_START\n"); */
            if (PADV_TX_READY == g_padv_radio.padv_tx_status) {
                break;
            }
            padv_exit_rx();
            vble_adv_tx_init(&u_pa_tx);
            vble_adv_tx_open(); //打开广播tx
            ret = audio_adc_init_api(RA_ENC_SR, AUDIO_ADC_MIC, 0);
            if (ret != 0) {
                log_error("audio_adc_init err 0x%x\n", ret);
                break;
            }
            sp_padv_enc_obj = audio2rf_encoder_io(RA_ENC_FUNC, &g_padv_radio.speaker_queue, NULL);
            if (NULL != sp_padv_enc_obj) {
                audio2rf_start_cmd(&g_padv_radio.speaker_queue, sp_padv_enc_obj->info.sr, sp_padv_enc_obj->info.br, RA_ENC_TYPE);
                audio2rf_encoder_start((enc_obj *)sp_padv_enc_obj);
                g_padv_radio.padv_tx_status = PADV_TX_READY;
            } else {
                padv_exit_tx(NEED_WAIT);
                log_error("tx_encoder_init_err\n");
            }
            break;
        case MSG_SENDER_STOP: //按键松开
            log_info("MSG_SENDER_STOP\n");
            padv_exit_tx(NEED_WAIT);
            break;
        case MSG_SET_RX_STATUS: //开关扫描
            if (PADV_TX_READY == g_padv_radio.padv_tx_status) {
                padv_exit_tx(NEED_WAIT);
            }
            if (PADV_RX_IDLE == g_padv_radio.padv_rx_status) {
                log_info("rx_init\n");
                vble_adv_rx_init(&u_pa_rx);
                log_info("rx_open\n");
                vble_adv_rx_open();
                g_padv_radio.padv_rx_status = PADV_RX_READY;
            } else {
                padv_exit_rx();
            }
            break;

        case MSG_CHANGE_WORK_MODE:
            log_info("MSG_CHANGE_WORK_MODE\n");
            padv_exit_rx();
            if (PADV_TX_IDLE != g_padv_radio.padv_tx_status) {
                padv_exit_tx(NEED_WAIT);
                break;
            }
            goto __padv_radio_loop_exit;
        case MSG_500MS:
            /* putchar('5'); */
            if (PADV_TX_READY == g_padv_radio.padv_tx_status) {
                audio2rf_start_cmd(&g_padv_radio.speaker_queue, sp_padv_enc_obj->info.sr, sp_padv_enc_obj->info.br, RA_ENC_TYPE);
            }
            if ((PADV_TX_IDLE == g_padv_radio.padv_tx_status) &&
                (PADV_RX_IDLE == g_padv_radio.padv_rx_status)) {
                audio_off();
                sys_power_down(-2);
                audio_init();
            }
        default:
            ap_handle_hotkey(msg[0]);
            break;
        }
    }
__padv_radio_loop_exit:
    vble_adv_rx_cb_register(NULL, NULL, NULL);
    queue2vble_tx_send_register(NULL);
}


/*----------------------------------------------------------------------------*/
/**@brief   周期广播式对讲机应用主函数
   @param
   @return
   @author
   @note    主函数根据不同状态运行不同子函数；
*/
/*----------------------------------------------------------------------------*/
void rf_radio_padv_app(void)
{

    rf_radio_padv_init();       //广播蓝牙初始化
    audio_init();

    padv_app_init();

    memset(&g_padv_radio, 0, sizeof(g_padv_radio));
    memset(&g_padv_dec,   0, sizeof(g_padv_dec));
    memset(&padv_packet,  0, sizeof(padv_packet));

    rf_radio_padv_loop();

    rf_radio_padv_uninit();
}


#endif



