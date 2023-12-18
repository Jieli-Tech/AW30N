#pragma bss_seg(".ar_send.data.bss")
#pragma data_seg(".ar_send.data")
#pragma const_seg(".ar_send.text.const")
#pragma code_seg(".ar_send.text")
#pragma str_literal_override(".ar_send.text.const")

#include "audio2rf_send.h"
#include "audio_rf_mge.h"
#include "app_modules.h"
#include "circular_buf.h"
#include "cpu.h"
#include "config.h"
#include "typedef.h"
#include "vfs.h"
#include "msg.h"
#include "crc16.h"
#include "trans_packet.h"

#include "encoder_mge.h"
#include "app.h"
#include "audio.h"
#include "audio_adc_api.h"
#include "audio_adc.h"
#include "audio_dac.h"

#if ENCODER_UMP3_EN
#include "ump3_encoder.h"
#endif

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[rf_send]"
#include "log.h"

/* 内部变量定义 */
volatile u32 rf_send_cnt AT(.ar_trans_data);
/* 外部变量/函数声明 */
extern enc_obj *enc_hdl;

/*----------------------------------------------------------------------------*/
/**@brief   发送编码头信息包给远端
   @param   sr      :编码采样率
            br      :编码码率
            enc_type:编码对应的解码类型
   @return  0:成功   非0:失败，错误值请查看errno-base.h
   @author
   @note    远端接收编码信息包后启动解码
*/
/*----------------------------------------------------------------------------*/
u32 rf_enc_head_output(u32 sr, u32 br, u8 enc_type)
{
    rf_send_cnt = 0;

    u8 tmp_buf[(sizeof(RF_RADIO_PACKET) + sizeof(RF_RADIO_ENC_HEAD))];
    memset(&tmp_buf[0], 0, sizeof(tmp_buf));

    RF_RADIO_ENC_HEAD enc_head = {0};
    enc_head.enc_type = enc_type;
    enc_head.reserved = 0xff;
    enc_head.sr = sr;
    enc_head.br = br;
    return audio2rf_send_packet(AUDIO2RF_START_PACKET, &enc_head, sizeof(enc_head));
}

/*----------------------------------------------------------------------------*/
/**@brief   发送停止包给远端
   @param
   @return  0:成功   非0:失败，错误值请查看errno-base.h
   @author
   @note    远端接收停止包后停止解码
*/
/*----------------------------------------------------------------------------*/
u32 audio2rf_send_stop_packet(void)
{
    log_info("********* SEND STOP *********\n");
    RF_RADIO_PACKET stop_packet = {0};
    return audio2rf_send_packet(AUDIO2RF_STOP_PACKET, NULL, 0);
}

/*----------------------------------------------------------------------------*/
/**@brief   编码器输出数据并进行无线传输
   @param   priv:私有结构体，此处为编码句柄
            data:输出数据buff
            len :输出数据长度
   @return  告知编码器本次输出的实际长度
   @author
   @note    无线传输模块出错导致无法正常发送时停止编码，发送繁忙时等待下次编码输出
*/
/*----------------------------------------------------------------------------*/
u32 rf_enc_output(void *priv, u8 *data, u16 len)
{
    enc_obj *obj = priv;
    if (NULL == obj) {
        return 0;
    }

    if (obj->enable & B_ENC_INIT) {
        /* 忽略部分编码格式输出的头信息 */
        return len;
    }

    u32 err = audio2rf_send_packet(AUDIO2RF_DATA_PACKET, data, len);
    if (0 != err) {
        if (E_AU2RF_RF_BUSY != err) {
            obj->enable |= B_ENC_FULL;
            post_event(EVENT_WFILE_FULL);
        }
        return 0;
    } else {
        return len;
    }
}

void audio2rf_encoder_stop(void)
{
    stop_encode_phy(ENC_NO_WAIT, 0);
}

/*----------------------------------------------------------------------------*/
/**@brief   发送编码信息并启动编码器
   @param   enc_fun     :编码器选择，可传入ump2或者opus编码器初始化接口
            input_func  :编码器输入回调
            output_func :编码器输出回调
            enc_type    :编码对应的解码类型选择
   @return
   @author
   @note    初始化编码器后会将编码信息发送给远端，再将编码链路挂载到AUDIO_ADC中断
            调用该函数前需确保AUDIO_ADC已工作
*/
/*----------------------------------------------------------------------------*/
bool audio2rf_encoder_io(u32(*enc_fun)(void *, void *, void *), void *input_func, void *output_func, u8 enc_type)
{
    /* if (false == audio_adc_enable_check()) { */
    /*     audio_adc_init_api(sr, AUDIO_ADC_MIC, 0) */
    /* } */
    rec_phy_init();
    enc_hdl = (void *)enc_fun(NULL, input_func, output_func);
    if (NULL == enc_hdl) {
        log_info("rf_encode fail \n");
        return false;
    }
    rf_enc_head_output(enc_hdl->info.sr, enc_hdl->info.br, enc_type);
    HWI_Install(IRQ_SOFT1_IDX, (u32)enc_soft1_isr,  IRQ_ENCODER_IP) ;
#if RF_SENDER_USE_SOFT_ISR
    HWI_Install(IRQ_SOFT2_IDX, (u32)rf_send_soft2_isr, IRQ_WFILE_IP) ;
#endif
    enc_hdl->enable = B_ENC_ENABLE;
    start_encode();
    log_info("rf_encode succ \n");
    return true;
}
