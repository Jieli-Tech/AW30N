#ifndef __if_sbc_dec_ctrl_h
#define __if_sbc_dec_ctrl_h


//typedef unsigned char	u8;
//typedef unsigned short	u16;
//typedef signed int      u32;
#include "typedef.h"

#define SET_DECODE_MODE   0x80
#define SET_DEC_CH_MODE   0x81       //配置声道模式参数
#define SET_FORMAT_MSBC   0x88
#define SET_FORMAT_USBC   0x89       //usbc模式,同时设置参数.  检查返回值为0,设置成功.


//ERROR_CODE for sbc.m dec
#define ERR_ERROR_NODATA         0x60    //本轮run读不到数据.
#define ERR_CANNOT_SYNC_TSLOOP   0x61    //本轮没有找到同步字
#define ERR_FRAMELEN_GT_BUFFSZ   0x62    //压缩帧长超出范围
//流媒体解码过程返回错误忽略 继续解码.


//typedef struct sbc_decoder_io {
//	void *priv;
//	int(*input)(void *priv, void **buf);
//	int(*output)(void *priv, void *data, int len);
//}SBC_DECODER_IO;


//run_errinfo
enum {
    NO_ERR,
    ERR_NO_BIT_STREAM,
    ERR_UPBUFF_LENLEZERO,
    ERR_UPBUFF_LENGTMAX,
    ERR_UPBUFF_PTRNULL,
};

//typedef struct sbc_decoder_inf {
//	u16 sr;            ///< sample rate
//	u16 br;            ///< bit rate
//	u32 nch;           ///<声道
//} sbc_dec_inf_t;
//


typedef struct _AUDIO_DEC_CH_OPUT_PARA {
    u32 mode;    //0:old_LRLRLR  1:LLLLLL  2:RRRRRR  3:(L*p0+R*p1)/16384
    short pL;    //mode_3_channel_L_coefficient  Q13  8192表示合成数据中对应通道音量为50%
    short pR;    //mode_3_channel_R_coefficient  Q13
} AUDIO_DEC_CH_OPUT_PARA;


// typedef struct _AUDIO_DECODE_PARA {
// u32 mode;
// } AUDIO_DECODE_PARA;   //设置是否完全输出方式.

typedef struct _USBC_DECODE_PARA {      //SET_FORMAT_USBC
    u32 sr;      //samplerate: 16000/32000/44100/48000.
    u8 bitpool;  //[4,15*subbands]  .bitpool影响码率.
    u8 subbands; //4|8   子带数
    u8 blocks;   //[4,16] .块长度.
    u8 snr;      //0|1   影响内部码流分配.  allocation.
    u8 nch;      //1|2 channels.
    u8 joint;    //0|1 joint_stereo  for nch=2.
    u8 dual;     //0|1 dualchannel   for nch=2,设1码率加倍.    joint和dual不能同时为1.
} USBC_DECODE_PARA;   //设置usbc解码参数.

//typedef struct _SBC_ENC_PARA_
//{
//	u32 sr;      //samplerate: 16000/32000/44100/48000.
//	u8 msbc;     //设为1,格式为msbc. 其他参数失效.  msbc:sr=16000,nch=1,bitpool=26,subbands=8,blocks=15.
//	u8 bitpool;  //[4,15*subbands]  .bitpool影响码率.  蓝牙常用单声道35  双声道51|53  at subbands=8.   subbands=4时参数应减半.
//	u8 subbands; //4|8   子带数.
//	u8 blocks;   //4/8/12/16  .块长度.
//	u8 snr;      //0|1   影响内部码流分配.  allocation.
//	u8 nch;      //1|2 channels.
//	u8 joint;    //0|1 joint_stereo  for nch=2.
//	u8 dual;     //0|1 dualchannel   for nch=2,设1码率加倍.    joint和dual不能同时为1.
//	//每帧每个声道输入pcm点数为  subbands*blocks.     GET_ENC_PCMPOINT_PERCH
//}SBC_ENC_PARA;
////add msbc = 2 for mono_usbc,   blocks参数支持4--16.  fixed nch=1,joint/dual失效.




/*------------------------------------------------------------------------------------------------*/
/**@brief   run   sbc解码主循环
   @param	u8 *ibuf    : 输入buffer地址
   @param	short *obuf : 输出buffer地址
   @param	int ilen    : 输入buffer长度(单位：bytes)
   @param	int olen    : 输出buffer长度(单位：bytes)
   @param	int *errinfo: 解码错误类型: ERR_NO_BIT_STREAM：位流耗尽   ERR_OBUF_LIMIT：输出buffer不够
   @return  解码输出bytes,单声道会copy一份到右声道  数据格式:LRLRLRLR……
   @note    u32(*run)(void *work_buf,u8 *ibuf,short *obuf,int ilen,int olen,int *errinfo)
*/

/*------------------------------------------------------------------------------------------------*/
/**@brief   get_frame_info  获取压缩及解码后的帧长(确保输入正确的sbc数据)
   @param	u8 *info    : 正确的sbc位流:4bytes
   @param	u16 *encfl  : sbc数据帧长度(单位：bytes)
   @param	u16 *decblk : sbc解码出数据块长度(单位：bytes)   一帧sbc最多解码出512bytes数据.
   @return  0:帧同步ok   -1:帧同步错误
   @note    int (*get_frame_info)(u8 *info,u16 *encfl,u16 *decblk)
*/

//typedef struct _sbc_audio_decoder_ops {
//	u32(*need_buf_size)() ;		                                      ///<获取解码需要的buffer
//	u32(*open)(void *work_buf, const SBC_DECODER_IO *decoder_io);     ///<打开解码器,初始化
//	u32(*run)(void *work_buf,int *errinfo);	           ///<主循环     //正常情况返回0, 需要更新输入buffer 返回1.
//	sbc_dec_inf_t *(*get_dec_inf)(void *work_buf) ;	   ///<获取解码信息
//	int (*get_frame_info)(u8 *info,u16 *encfl,u16 *decblk);
//	u32(*dec_config)(void *work_buf, u32 cmd, void*parm);   ///配置声道模式参数  获取淡入淡出完成状态  设置非0声道模式输出通道数
//} sbc_audio_decoder_ops;


// struct if_decoder_io {
// void* priv;
// int (*input)(void* priv, u32 addr, void* buf, int len, u8 type);
/*
priv -- 私有结构体，由初始化函数提供。
addr -- 文件位置
buf  -- 读入地址
len  -- 读入长度 512 的整数倍
type -- 0 --同步读（等到数据读回来，函数才返回） ，1 -- 异步读（不等数据读回来，函数就放回）

*/
// int (*check_buf)(void* priv, u32 addr, void* buf);
// u32(*output)(void* priv, void* data, int len);
// u32(*get_lslen)(void* priv);
// u32(*store_rev_data)(void* priv, u32 addr, int len);
// };
// typedef struct if_decoder_io IF_DECODER_IO;



// typedef struct decoder_inf {
// u16 sr;            ///< sample rate
// u16 br;            ///< bit rate
// u32 nch;           ///<声道
// u32 total_time;    ///<总时间
// } dec_inf_t;


// typedef struct __audio_decoder_ops {
// char* name;                                                            ///< 解码器名称
// u32(*open)(void* work_buf, const struct if_decoder_io* decoder_io, u8* bk_point_ptr);   ///<打开解码器

// u32(*format_check)(void* work_buf);					///<格式检查

// u32(*run)(void* work_buf, u32 type);				///<主循环

// dec_inf_t* (*get_dec_inf)(void* work_buf);			///<获取解码信息
// u32(*get_playtime)(void* work_buf);					///<获取播放时间
// u32(*get_bp_inf)(void* work_buf);					///<获取断点信息

// u32(*need_dcbuf_size)();		                     ///<获取解码需要的buffer
// u32(*need_rdbuf_size)();                             ///<获取解码读数buf的读文件缓存buf的大小
// u32(*need_bpbuf_size)();				             ///<获取保存断点信息需要的buffer


// void (*set_step)(void* work_buf, u32 step);			 ///<设置快进快进步长。
// void (*set_err_info)(void* work_buf, u32 cmd, u8* ptr, u32 size);		///<设置解码的错误条件
// u32(*dec_config)(void* work_buf, u32 cmd, void* parm);
// } audio_decoder_ops, decoder_ops_t;


extern int get_sbc_frame_info(u8 *info, u16 *encfl, u16 *decblk);
extern audio_decoder_ops *get_sbc_stdec_ops();


//#pragma bss_seg(".sbc_stddec.data.bss")
//#pragma data_seg(".sbc_stddec.data")
//#pragma const_seg(".sbc_stddec.text.const")
//#pragma code_seg(".sbc_stddec.text")



#endif



