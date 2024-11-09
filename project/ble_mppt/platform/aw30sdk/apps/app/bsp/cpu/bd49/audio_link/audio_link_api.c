#include "app_modules.h"
#include "app_config.h"
#include "audio_link.h"
#include "audio_link_sync.h"
#include "iis.h"
#include "gpio.h"

#include "circular_buf.h"
#include "sound_kick.h"
#include "audio_adc.h"
#if 1
#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[normal]"
#include "log.h"




ALINK_PARM	alink_mge = {
    .mclk_io 		= 	ALINK_MCLK_IO,
    .sclk_io  		= 	ALINK_SCLK_IO,
    .lrclk_io 		= 	ALINK_LCLK_IO,
    .mode 			= 	ALINK_MODE_TYPE,		//IIS模式
    .role 			= 	ALINK_ROLE_TYPE,		//主机
    .clk_mode 		= 	ALINK_CLK_MODE_TYPE,
    .bitwide 		= 	ALINK_BIT_WIDE_TYPE,
    .sclk_per_frame = 	ALINK_SCLK_PER_FRAME_TYPE,
    .dma_len 		= 	ALINK_DMA_LEN,			//16bit : 64 * 2(CH) * 2(Byte)* 2(dual buffer)
    .sample_rate 	= 	ALINK_SR_DEFAULT,		//SR
};




u8 alink_chnl[ALINK_MAX_CHANNEL];	//标记4个通道哪个正在使用
u16 alink_sync_timer_id;



int audio_link_en_api(enum ALINK_MODULE_EN en)
{
    return audio_link_en(en);
}


u32 audio_link_init_api(void)
{
    u32 ret = 0;
    memset(alink_chnl, -1, sizeof(alink_chnl));
    ret = alink_init(&alink_mge);
    log_info("audio_link_init_api ret %d\n", ret);
    alink_sync_timer_id = sys_timer_add(NULL, audio_link_sync_once, 20);

    return ret;
}


u32 audio_link_uninit_api(void)
{
    u32 ret = 0;
    ret = alink_uninit(&alink_mge);
    memset(alink_chnl, -1, sizeof(alink_chnl));
    sys_timer_del(alink_sync_timer_id);
    return ret;
}


u16 audio_link_set_sr_api(u32 rate)
{
    u16 ret = alink_sr(rate);
    if (ret != 0) {
        alink_mge.sample_rate = ret;
    }
    return ret;
}


u16 read_audio_link_sr()
{
    return alink_mge.sample_rate;
}


u32 audio_link_init_channel_api(ALINK_CH_CFG *alink_data_parm)
{
    u8 alink_chnl_idx = (u8)(-1);
    for (u8 i = 0; i < ALINK_MAX_CHANNEL; i++) {
        if (alink_chnl[i] != (u8) - 1) {
            continue;
        } else {
            alink_chnl_idx = i;
            alink_chnl[i] = 1;
            break;
        }
    }
    if (alink_chnl_idx == (u8)(-1)) {
        log_error("audio link chnl all use!!\n");
        return -1;
    }
    /* log_info("alink_chnl_idx %d\n",alink_chnl_idx); */
    ALINK_CH_CFG *p_alink_data_parm = alink_data_parm;
    p_alink_data_parm->ch_idx = alink_chnl_idx;	//查询完把空的通道序号记录在data_parm

    u32 ret = alink_channel_init(p_alink_data_parm);	//配置data通道内容
    if (ret == (u8) - 1) {
        log_error("alink_channel_init fail 0x%x\n", ret);
        alink_chnl[alink_chnl_idx] = -1;
        p_alink_data_parm->ch_idx = -1;
        return ret;
    }


    /* 同步 */
    audio_link_sync_init(alink_chnl_idx, ALINK_SR_DEFAULT);

    return ret;
}


u32 audio_link_uninit_channel_api(ALINK_CH_CFG *alink_data_parm)
{
    alink_chnl[alink_data_parm->ch_idx] = (u8) - 1;
    alink_channel_uninit(alink_data_parm);
    return 0;
}


#endif
