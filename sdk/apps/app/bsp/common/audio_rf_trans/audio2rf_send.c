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
#include "rf_send_queue.h"

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
volatile u16 packet_type_cnt[2] AT(.ar_trans_data);
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
u32 audio2rf_start_cmd(u32 sr, u32 br, AUDIO_FORMAT enc_type)
{
    /* rf_send_cnt = 0; */

    /* memset((u8 *)&packet_type_cnt[0], 0, sizeof(packet_type_cnt)); */

    RF_RADIO_ENC_HEAD enc_head = {0};
    enc_head.enc_type = enc_type;
    enc_head.reserved = 0xff;
    enc_head.sr = sr;
    enc_head.br = br;
    return audio2rf_send_packet(AUDIO2RF_START_PACKET, (u8 *)&enc_head, sizeof(enc_head));
}

/*----------------------------------------------------------------------------*/
/**@brief   发送停止包给远端
   @param
   @return  0:成功   非0:失败，错误值请查看errno-base.h
   @author
   @note    远端接收停止包后停止解码
*/
/*----------------------------------------------------------------------------*/
u32 audio2rf_stop_cmd(void)
{
    log_info("********* SEND STOP *********\n");
    return audio2rf_send_packet(AUDIO2RF_STOP_PACKET, NULL, 0);
}

/*----------------------------------------------------------------------------*/
/**@brief   回复指定CMD的ACK包给远端
   @param   ack_cmd :需要回复ack的命令包
            data    :ack参数buff
            len     :ack参数长度
   @return  0:成功   非0:失败，错误值请查看errno-base.h
   @author
   @note    回复指定cmd命令的ack包，ack包的type会置上AUDIO2RF_ACK
*/
/*----------------------------------------------------------------------------*/
u32 audio2rf_ack_cmd(u8 ack_cmd, u8 *data, u16 len)
{
    log_info("********* PACKET_TYPE %d, SEND ACK:%d *********\n", ack_cmd, data[0]);
    return audio2rf_send_packet(ack_cmd | AUDIO2RF_ACK, data, len);
}

