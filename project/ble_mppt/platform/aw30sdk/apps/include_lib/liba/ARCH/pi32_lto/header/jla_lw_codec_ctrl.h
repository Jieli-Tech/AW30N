


#ifndef __jla_lw_ctrl_h
#define __jla_lw_ctrl_h


#include "typedef.h"


#define  RESET_PACKET         1    //重置包内帧记录,new_packet_start.
#define  SET_FADE_IN_OUT      0x82    //淡入淡出  设置生效.



struct jla_lw_codec_param {
    int  nframes;    //n frames per packet.     1~16.
    struct ujlalw_frame_attribute {
        unsigned char  bitpool;   //4~120.
        unsigned char  blocks;    //4~16.
    } frame_set[16];
};



struct jla_lw_codec_ops {
    u32(*need_buf)(void);
    u32(*init)(void *ptr, void *info);  //run_buf +  struct jla_lw_codec_param
    u32(*encode)(void *ptr, s16 *indata, u8 *outdata, s16 len);
    u32(*decode)(void *ptr, u8 *indata, s16 *outdata, s16 len);
    u32(*get_frame_len)(int bitpool, int blks);
    u32(*codec_config)(void *ptr, u32 cmd, void *parm);
};


extern struct jla_lw_codec_ops *get_jla_lw_ops();

extern const int JLA_LW_PLC_EN;
extern const int JLA_LW_PLC_FADE_OUT_START_POINT;
extern const int JLA_LW_PLC_FADE_OUT_POINTS;
extern const int JLA_LW_PLC_FADE_IN_POINTS;



/*
 * 自定义JLA_LW格式编解码说明
 *
 * 通用参数 void *ptr : 运行buffer.
 *
 * FRLEN = get_frame_len().       根据参数计算出压缩数据帧长.  待编码或者解码后的pcm数据长度为blks*8点.
 * bitpool=[4:120],blks=[4:16].   bitpool=26为mjlalw音质.
 *
 * init返回值FRLEN   默认bitpool=35,blks=16.  主要将包内部帧参数struct jla_lw_codec_param *info配置进去,编解码对应帧时更新参数.
 *
 * Tips：
 *      bitpool/blks ---> FRLEN，为encode编码完成一帧的输出长度. 也是对应参数decode输入一帧的长度，字节数. 可通过get_frame_len获取.
 *      bitpool/blks ---> FRBLK = blks*8点，为encode编码一帧输入数据的长度，也是对应参数decode输出一帧的长度.
 *      encode/decode  输入完整帧数据.
 *
 * encode / decode  编/解码一帧数据.   返回值为输出数据长度(字节)   解码异常会填充数据0.
 * indata  : 输入数据buffer
 * outdata : 输出数据buffer
 * len     : 输入数据长度,字节数. 需要完整帧数据.
 *
 *
 */


//#pragma bss_seg(".jla_lw_codec_bss")
//#pragma data_seg(".jla_lw_codec_data")
//#pragma const_seg(".jla_lw_codec_const")
//#pragma code_seg(".jla_lw_codec_code")



#endif












