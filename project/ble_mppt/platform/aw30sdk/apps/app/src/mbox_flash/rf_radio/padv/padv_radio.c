
#include "trans_packet.h"
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
#include "rf_radio_app.h"
#include "rf_radio_rs.h"
#if defined (PADVB_WL_MODE) && (PADVB_WL_MODE)
#include "third_party/padvb_intercom/padvb.h"
#include "encoder_stream.h"

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[padv_radio]"
#include "log.h"

#if ENCODER_JLA_LW_EN
#define PADV_ENC_SR      32000
#define PADV_ENC_TYPE    FORMAT_JLA_LW
#define PADV_ENC_FUNC    jla_lw_encode_api
#else
#define PADV_ENC_SR      0
#define PADV_ENC_TYPE    -1
#define PADV_ENC_FUNC    NULL
#endif

#define MSG_SET_RX_STATUS MSG_BLE_ROLE_SWITCH //节省资源，直接使用传统对讲机的按键分布

/* 使用双方约定好的编码参数做解码 */
/* 该方式在同步后可以立刻开启解码，但是参数配置方面不够灵活*/
/* void padv_radio_post_start_cmd() */
/* { */
/* RF_RADIO_ENC_HEAD enc_head = {0}; */
/* enc_head.enc_type = PADV_ENC_TYPE; */
/* enc_head.reserved = 0xff; */
/* enc_head.sr = PADV_ENC_SR; */
/* enc_head.br = 72; */
/* packet_cmd_post(radio_mge.packet_recv.cmd_pool, AUDIO2RF_START_PACKET, (u8 *)&enc_head, sizeof(enc_head)); */
/* } */



dec_obj *padv_decode_start(RF_RADIO_ENC_HEAD *p_enc_head)
{
    log_info("rra_dec_init\n");
    if (NULL != radio_mge.packet_recv.dec_obj) {
        log_error("rf dev_obj not null ");
    }
    dec_obj *p_dec_obj = rra_decode_phy(p_enc_head);
    if (NULL == p_dec_obj) {
        memset(&radio_mge.stream, 0, sizeof(sound_stream_obj));
    }
    return p_dec_obj;
}

static RF_RADIO_ENC_HEAD s_enc_head;
void padv_decode_stop(void)
{
    log_info("rra_dec_stop\n");
    if (NULL != radio_mge.packet_recv.dec_obj) {
        rf2audio_decoder_stop(radio_mge.packet_recv.dec_obj, unregist_dac_channel);
        radio_mge.packet_recv.dec_obj = NULL;
        radio_mge.stream.p_ibuf = NULL;
        memset(&s_enc_head, 0, sizeof(s_enc_head));
    }
    dac_off_api();
}


void padv_radio_post_stop_cmd()
{
    packet_cmd_post(radio_mge.packet_recv.cmd_pool, AUDIO2RF_STOP_PACKET, NULL, 0);
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
    rev_fsm_mge *p_recv_ops = &radio_mge.packet_recv;
    u32 len = packet_cmd_get(p_recv_ops->cmd_pool, &rra_packet[0], sizeof(rra_packet));
    if (0 == len) {
        return E_PACKET_NULL;
    }
    switch (rra_packet[1]) {
    case AUDIO2RF_START_PACKET:
        log_info("AUDIO2RF_START_PACKET\n");
        if (PADV_TX_IDLE != radio_mge.padv_tx_status) {
            log_info("encoding\n");
            break;
        }
        if (PADV_RX_IDLE == radio_mge.padv_rx_status) {
            log_info("rx_no_ready, cannot_decode\n");
            break;
        }
        RF_RADIO_ENC_HEAD *p_enc_head = (RF_RADIO_ENC_HEAD *)&rra_packet[2];
        if (NULL != radio_mge.packet_recv.dec_obj) {
            u32 res = memcmp(p_enc_head, &s_enc_head, sizeof(s_enc_head));//防止没有stop包
            if (0 == res) {
                break;
            }
            padv_decode_stop();

        }
        memcpy(&s_enc_head, p_enc_head, sizeof(s_enc_head));
        radio_mge.packet_recv.dec_obj = padv_decode_start(p_enc_head);
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
    if (PADV_RX_IDLE != radio_mge.padv_rx_status) {
        log_info("rx_close\n");
        vble_adv_rx_close();
        log_info("rx_uninit\n");
        vble_adv_rx_uninit();
    }

    log_info("rra_dec_stop\n");
    padv_decode_stop();
    radio_mge.padv_rx_status = PADV_RX_IDLE;
    //-----
    /* rra_timeout_reset(); */
}

/* #include "encoder_mge.h" */
void padv_exit_tx(ENC_STOP_WAIT wait)
{
    log_info("rra_enc_stop\n");
    if (NULL != radio_mge.enc_obj) {
        audio2rf_encoder_stop(wait);
        u32 retry = 50;
        while ((0 != get_queue_data_size()) && (0 != retry)) {
            /* 设置的广播间隔可能较长 */
            delay_10ms(2);
            retry--;
        }		
		rf_send_soft_isr_init(NULL, 0);
        delay_10ms(20);		
        radio_mge.enc_obj = NULL;
    }
    audio_adc_off_api();

    vble_adv_tx_close(); //为了消耗完编码数据,tx放最后关
    vble_adv_tx_uninit();

    radio_mge.padv_tx_status = PADV_TX_IDLE;
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
bool padv_rrapp_loop()
{
    log_info("__padv_rrapp_loop__\n");
    vble_padv_param_init();
    vble_adv_rx_cb_register(
        &radio_mge.packet_recv,
        (void (*)(void))padv_radio_post_stop_cmd,
        (int (*)(void *, u8 *, u16))unpack_data_deal
    );
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
            if (PADV_TX_READY == radio_mge.padv_tx_status) {
                break;
            }
            padv_exit_rx();
            vble_adv_tx_init(&u_pa_tx);
            vble_adv_tx_open(); //打开广播tx
            ret = audio_adc_init_api(PADV_ENC_SR, AUDIO_ADC_MIC, 0);
            if (ret != 0) {
                log_error("audio_adc_init err 0x%x\n", ret);
                break;
            }
            radio_mge.enc_obj = audio2rf_encoder_io(PADV_ENC_FUNC, PADV_ENC_TYPE);
            rf_send_soft_isr_uninit();
            if (NULL != radio_mge.enc_obj) {
                audio2rf_start_cmd(radio_mge.enc_obj->info.sr, radio_mge.enc_obj->info.br, PADV_ENC_TYPE);
                rf_send_soft_isr_init(radio_mge.enc_obj->p_obuf, 0);
                audio2rf_encoder_start((enc_obj *)radio_mge.enc_obj);
                radio_mge.padv_tx_status = PADV_TX_READY;
            } else {
                padv_exit_tx(ENC_NEED_WAIT);
                log_error("tx_encoder_init_err\n");
            }
            break;
        case MSG_SENDER_STOP: //按键松开
            log_info("MSG_SENDER_STOP\n");
            padv_exit_tx(ENC_NEED_WAIT);
            break;
        case MSG_SET_RX_STATUS: //开关扫描
            if (PADV_TX_READY == radio_mge.padv_tx_status) {
                padv_exit_tx(ENC_NEED_WAIT);
            }
            if (PADV_RX_IDLE == radio_mge.padv_rx_status) {
                log_info("rx_init\n");
                vble_adv_rx_init(&u_pa_rx);
                log_info("rx_open\n");
                vble_adv_rx_open();
                radio_mge.padv_rx_status = PADV_RX_READY;
            } else {
                padv_exit_rx();
            }
            break;

        case MSG_CHANGE_WORK_MODE:
            log_info("MSG_CHANGE_WORK_MODE\n");
            padv_exit_rx();
            if (PADV_TX_IDLE != radio_mge.padv_tx_status) {
                padv_exit_tx(ENC_NEED_WAIT);
                break;
            }
            radio_mge.rra_mode = RRA_EXIT;
            goto __padv_radio_loop_exit;
        case MSG_500MS:
            /* putchar('5'); */
            if (PADV_TX_READY == radio_mge.padv_tx_status) {
                /* putchar('e'); */
                audio2rf_start_cmd(radio_mge.enc_obj->info.sr, radio_mge.enc_obj->info.br, PADV_ENC_TYPE);
            }
            if ((PADV_TX_IDLE == radio_mge.padv_tx_status) &&
                (PADV_RX_IDLE == radio_mge.padv_rx_status)) {
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
    /* vble_adv_rx_cb_register(NULL, NULL, NULL, NULL); */
    return radio_mge.rra_mode;
}


void padv_app_init(void)
{
    memset(&s_enc_head, 0, sizeof(s_enc_head));
    radio_mge.rra_mode = RRA_PERIOD_ADV; //所有消息都先去到这个分支处理
    radio_mge.padv_tx_status = PADV_TX_IDLE;
    radio_mge.padv_rx_status = PADV_RX_IDLE;
}





#endif



