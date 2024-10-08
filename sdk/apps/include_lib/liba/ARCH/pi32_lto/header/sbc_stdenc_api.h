﻿
#ifndef _SBC_STDENC_API_H_
#define _SBC_STDENC_API_H_

#ifndef u16
#define  u32  unsigned int
#define  u16  unsigned short
#define  s16  short
#define  u8  unsigned char
#endif


//ERROR_CODE for sbc.m enc
#define ERR_ERROR_NODATA       0x60     //读不够数据.
#define ERR_SET_INFO_SR_CHM    0x61     //错误的采样率/声道模式信息
#define ERR_BITPOOL_SUBBAND    0x62     //错误的bitpool/subband信息
#define ERR_BSC_COMBINATION    0x63     //错误的bitpool+suband+channel mode组合
#define ERR_GOALFLEN_OVFLOW    0x64     //目标帧长超出范围
//编码open时判断参数是否满足需求    返回0成功.
//run 不会返回错误的.



typedef struct _SBC_ENC_FILE_IO_ {
    void *priv;
    u16(*input_data)(void *priv, s16 *buf, u8 channel, u16 len); //short
    u32(*output_data)(void *priv, u8 *buf, u16 len); //bytes
} SBC_ENC_FILE_IO;


typedef struct _SBC_ENC_PARA_ {
    u32 sr;      //sbc/usbc amplerate: 16000/32000/44100/48000; msbc:16000
    u8 msbc;     //设为0,格式为sbc; 设为1,格式为msbc. 其他参数失效.  msbc:sr=16000,nch=1,bitpool=26,subbands=8,blocks=15. 设为2，格式为usbc
    u8 bitpool;  //[4,15*subbands]  .bitpool影响码率.  蓝牙常用单声道35  双声道51|53  at subbands=8.   subbands=4时参数应减半.
    u8 subbands; //4|8   子带数.
    u8 blocks;   //sbc:4/8/12/16; msbc:15; usbc:4~16; 块长度.
    u8 snr;      //0|1   影响内部码流分配.  allocation.
    u8 nch;      //1|2 channels.
    u8 joint;    //0|1 joint_stereo  for nch=2.
    u8 dual;     //0|1 dualchannel   for nch=2,设1码率加倍.    joint和dual不能同时为1.
    //每帧每个声道输入pcm点数为  subbands*blocks.     GET_ENC_PCMPOINT_PERCH
} SBC_ENC_PARA;
//add msbc = 2 for usbc,   blocks参数支持4--16.




#define  GET_ENC_PCMPOINT_PERCH   0x10     //cmd 获取每帧每个通道需要输入的pcm点数  (每个点2字节).
#define  GET_ENCODE_FRLEN         0x11     //cmd 获取压缩后帧长.

typedef struct __SBC_STENC_OPS {
    u32(*need_buf)(SBC_ENC_PARA *para);      //nch.
    u32(*open)(u8 *ptr, SBC_ENC_FILE_IO *audioIO, SBC_ENC_PARA *para);  //return 0 init ok.
    u32(*run)(u8 *ptr);
    u32(*run_wb)(u8 *ptr, short *pcmL, short *pcmR, u8 *obuf, int *encLen);
    u32(*config)(u8 *ptr, u32 cmd, void *parm);
} SBC_STENC_OPS;


extern SBC_STENC_OPS *get_sbc_stenc_ops(void);


//#pragma bss_seg(".sbc_stdenc.data.bss")
//#pragma data_seg(".sbc_stdenc.data")
//#pragma const_seg(".sbc_stdenc.text.const")
//#pragma code_seg(".sbc_stdenc.text")





#endif


