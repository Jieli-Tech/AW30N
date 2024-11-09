#pragma bss_seg(".encoder_api.data.bss")
#pragma data_seg(".encoder_api.data")
#pragma const_seg(".encoder_api.text.const")
#pragma code_seg(".encoder_api.text")
#pragma str_literal_override(".encoder_api.text.const")

#include "encoder_mge.h"
#include "audio_adc_api.h"
#include "cpu.h"
#include "config.h"
#include "typedef.h"
#include "hwi.h"
/* #include "dev_manage.h" */
/* #include "fs_io.h" */
/* #include "vfs.h" */
#include "circular_buf.h"
/* #include "a_encoder.h" */
/* #include "mp3_encoder.h" */
#include "clock.h"

#include "trans_packet.h"
#include "rf_send_queue.h"
#include "audio2rf_send.h"


#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[normal]"
#include "log.h"

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

    if (0 != obj->indata_kick_size) {
        /* ibuf数据大于一帧数据 且 蓝牙压包缓存大于一帧数据 */
        if (cbuf_get_data_size(obj->p_ibuf) >= obj->indata_kick_size) {
            kick_encode_isr();
        }
    }
    return len;
}

void enc_wait_stop_stream(enc_obj *obj)
{
    log_info("stop encode C\n");
    u32 i = 0x10000;
    while (0 != cbuf_get_data_size(obj->p_obuf) && (0 != i)) {
        if (obj->enable & B_ENC_FULL) {
            break;
        }
        kick_rf_queue_isr();
        delay(100);
        i--;
    }
    /* HWI_Uninstall(IRQ_SOFT2_IDX); */
}

/*----------------------------------------------------------------------------*/
/**@brief   发送编码信息并启动编码器
   @param   enc_fun     :编码器选择，可传入ump2或者opus编码器初始化接口
            enc_type    :编码对应的解码类型选择
   @return
   @author
   @note    初始化编码器后会将编码信息发送给远端，再将编码链路挂载到AUDIO_ADC中断
            调用该函数前需确保AUDIO_ADC已工作
*/
/*----------------------------------------------------------------------------*/
enc_obj *audio2rf_encoder_io(u32(*enc_fun)(void *, void *, void *),  AUDIO_FORMAT enc_type)
{

    /* enc_hdl = (void *)enc_fun(NULL, enc_input, rf_enc_output); */
    enc_obj *obj = encoder_io(enc_fun, enc_input, rf_enc_output, NULL);
    if (NULL == obj) {
        return NULL;
    }
    obj->wait_output_empty = (void *)enc_wait_stop_stream;
    /* #if RF_SENDER_USE_QUEUE */
    rf_send_soft_isr_init(obj->p_obuf, 1);
    /* #endif */
    /* audio2rf_start_cmd(obj->info.sr, obj->info.br, enc_type); */
    return obj;
}

u32 audio2rf_encoder_start(enc_obj *obj)
{
    if (NULL != obj) {
        obj->enable = B_ENC_ENABLE;
        start_encode();//adc_enable();
    }
    return 0;
}



void audio2rf_encoder_stop(ENC_STOP_WAIT wait)
{
    stop_encode_phy(wait);
    audio2rf_stop_cmd();
    HWI_Uninstall(IRQ_SOFT2_IDX);
}
