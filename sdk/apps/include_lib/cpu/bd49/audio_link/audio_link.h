#ifndef _AUDIO_LINK_H_
#define _AUDIO_LINK_H_

#include "typedef.h"
//ch_num
#define ALINK_CH0    	0
#define ALINK_CH1    	1
#define ALINK_CH2    	2
#define ALINK_CH3    	3

#define ALNK_BUF_POINTS_NUM 		64
#define ALINK_CLK_OUPUT_DISABLE 	0xFF
#define AUDIO_LINK_CHANNEL_TOTAL 	4
#define ALINK_TRACK_NUMBER   		2

//ch_dir
typedef enum {
    ALINK_DIR_TX	= 0u,
    ALINK_DIR_RX		,
} ALINK_DIR;

typedef enum {
    ALINK_MD_NONE	= 0u,
    ALINK_MD_IIS		, 		//IIS
    ALINK_MD_IIS_LALIGN	,		//左对齐
    ALINK_MD_IIS_RALIGN	,		//左对齐
    ALINK_MD_DSP0		,		//DSP0
    ALINK_MD_DSP1		,		//DSP1
} ALINK_MODE;


typedef enum {
    ALINK_ROLE_MASTER, //主机
    ALINK_ROLE_SLAVE,  //从机
} ALINK_ROLE;


typedef enum {
    ALINK_CLK_FALL_UPDATE_RAISE_SAMPLE, //下降沿更新数据, 上升沿采样数据
    ALINK_CLK_RAISE_UPDATE_FALL_SAMPLE, //上降沿更新数据, 下升沿采样数据
} ALINK_CLK_MODE;


typedef enum {
    ALINK_LEN_16BIT = 0u,
    ALINK_LEN_24BIT		, //ALINK_FRAME_MODE需要选择: ALINK_FRAME_64SCLK
} ALINK_DATA_WIDTH;


typedef enum {
    ALINK_FRAME_32SCLK, 	//32 sclk/frame
    ALINK_FRAME_64SCLK, 	//64 sclk/frame
} ALINK_FRAME_MODE;


typedef enum {
    ALINK_SR_48000 = 48077,		//48077
    ALINK_SR_44100 = 44117,		//44117
    ALINK_SR_32000 = 32051,		//32051
    ALINK_SR_24000 = 24038,		//24038
    ALINK_SR_22050 = 22059,		//22059
    ALINK_SR_16000 = 16026,		//16026
    ALINK_SR_12000 = 12019,		//12019
    ALINK_SR_11025 = 11029,		//11029
    ALINK_SR_8000  = 8013,		//8013
} ALINK_SR;


enum ALINK_MODULE_EN {
    ALINK_MODULE_CLOSE = 0u, 	//32 sclk/frame
    ALINK_MODULE_OPEN, 	//64 sclk/frame
};

typedef struct _ALNK_CH_CFG {
    u8 enable;
    u8 ch_idx;
    u8 data_io;					//data IO配置
    ALINK_DIR dir; 				//通道传输数据方向: Tx, Rx
    void *buf;					//dma buf地址
    void (*isr_cb)(u8 ch, u8 *buf, u32 len);
} ALINK_CH_CFG;


//===================================//
//多个通道使用需要注意:
//1.数据位宽需要保持一致
//2.buf长度相同
//===================================//
typedef struct _ALINK_PARM {
    // u8 port_select;
    u8 mclk_io; 				//mclk IO输出配置: ALINK_CLK_OUPUT_DISABLE不输出该时钟
    u8 sclk_io;					//sclk IO输出配置: ALINK_CLK_OUPUT_DISABLE不输出该时钟
    u8 lrclk_io;				//lrclk IO输出配置: ALINK_CLK_OUPUT_DISABLE不输出该时钟
    ALINK_CH_CFG ch_cfg[4];		//通道内部配置
    ALINK_MODE mode; 					//IIS, left, right, dsp0, dsp1
    ALINK_ROLE role; 					//主机/从机
    ALINK_CLK_MODE clk_mode; 			//更新和采样边沿
    ALINK_DATA_WIDTH  bitwide;   		//数据位宽16/32bit
    ALINK_FRAME_MODE sclk_per_frame;  	//32/64 sclk/frame
    u16 dma_len; 						//buf长度: byte
    ALINK_SR sample_rate;				//采样率
} ALINK_PARM;

u32 alink_init(ALINK_PARM *parm);
u32 alink_uninit(ALINK_PARM *parm);
u16 alink_sr(u32 rate);
u32 alink_channel_init(ALINK_CH_CFG *alink_data_parm);
u32 alink_channel_uninit(ALINK_CH_CFG *alink_data_parm);
int audio_link_en(enum ALINK_MODULE_EN en);

#endif
