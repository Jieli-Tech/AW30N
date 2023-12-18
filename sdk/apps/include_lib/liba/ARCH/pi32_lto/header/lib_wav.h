#ifndef __LIB_WAV_ARCH_H__
#define __LIB_WAV_ARCH_H__
//sh54
#include "if_decoder_ctrl.h"

#define WAV_DBUF_SIZE (2205)

#define WAV_TRACK    0 // B_STEREO


enum {
    FAST_L_OUT = 0x01,                  //输出左声道
    FAST_R_OUT = 0x02,                  //输出右声道
    FAST_LR_OUT = 0x04                 //输出左右声道混合
};

#define CMD_SET_DECODE_CH   0x91
typedef struct _PARM_DECODE_CHV_ {
    u32  ch_value;
} PARM_DECODE_CHV;



PARM_DECODE_CHV parm_nchv;

void wav_dconfig(decoder_ops_t *ops, void *dbuf)
{
    parm_nchv.ch_value = FAST_LR_OUT;
    ops->dec_confing(dbuf, CMD_SET_DECODE_CH, &parm_nchv);  //配置解码输出通道
}

#define WAV_DECODER_CONFIG(n)	wav_dconfig(n)




#endif
