/***********************************Jieli tech************************************************
  File : dac_api.c
  By   : liujie
  Email: liujie@zh-jieli.com
  date : 2019-1-14
********************************************************************************************/
#define __AUDIO_DAC_API

#pragma bss_seg(".audio_dac_api.data.bss")
#pragma data_seg(".audio_dac_api.data")
#pragma const_seg(".audio_dac_api.text.const")
#pragma code_seg(".audio_dac_api.text")
#pragma str_literal_override(".audio_dac_api.text.const")

#include "app_modules.h"
#include "audio_dac_api.h"
#include "audac_basic.h"
/* #include "circular_buf.h" */
#include "string.h"
#include "uart.h"
#include "config.h"
#include "audio.h"
/* #include "decoder_api.h" */
#include "audio_analog.h"
#include "clock.h"
/* #include "efuse.h" */
#include "audio_energy_detect.h"
#include "circular_buf.h"
#include "audio_dac_fade.h"
#include "app_config.h"
#include "audio_link/audio_link_sync.h"
#include "sound_mge.h"
#include "src.h"

#if HAS_MIO_EN
#include "mio_api.h"
#endif

/* #define LOG_TAG_CONST       NORM */
#define LOG_TAG_CONST       OFF
#define LOG_TAG             "[audio_dac_api]"
#include "log.h"

/* #include "printf.h" */


extern _OTP_CONST_  APA_CTRL_HDL apa_ops;


const u16 vol_tab[] = {
    0	,
    164	,
    191	,
    223	,
    260	,
    303	,
    353	,
    412	,
    480	,
    559	,
    652	,
    760	,
    887	,
    1034	,
    1205	,
    1405	,
    1638	,
    1910	,
    2227	,
    2597	,
    3028	,
    3530	,
    4115	,
    4798	,
    5594	,
    6523	,
    7605	,
    8867	,
    10338	,
    12053	,
    14052	,
    16384
};




#define MAX_VOL_LEVEL ((sizeof(vol_tab)/2) - 1)
#define MAX_PHY_VOL   vol_tab[MAX_VOL_LEVEL]

__attribute__((always_inline))
u32 get_dac_max_phy_vol(void)
{
    return MAX_PHY_VOL;
}

DAC_MANAGE dac_mge;



/* s16 sp_dac_buf[DAC_PACKET_SIZE] AT(.DAC_BUFFER); */
/*----------------------------------------------------------------------------*/
/**@brief   dac模块初始化
   @param   无
   @return  无
   @author  liujie
   @note    void dac_mode_init(u16 vol)
*/
/*----------------------------------------------------------------------------*/
void dac_mode_init(u16 vol)
{
    memset(&dac_mge, 0, sizeof(dac_mge));
#if (AUOUT_USE_DAC == AUOUT_USE_APA)
    dac_mge.flag |= B_DAC_DIG_VOL;
#endif
#if DAC_FADE_ENABLE

    if (vol > MAX_VOL_LEVEL) {
        vol = MAX_VOL_LEVEL;
    }
    dac_mge.vol_l = vol;
    dac_mge.vol_r = vol;

#else
    stereo_dac_vol(0, vol, vol);
#endif

    /* apa_clr_buf(); */
    /* apa_resource_init((void *)&apa_ops); */
    auout_mode_init();
}


void dac_init(u32 sr, u32 delay_flag)
{
    auout_init(sr, delay_flag);
    /* apa_phy_init(apa_sr_lookup(sr)); */
    /* if (delay_flag) { */
    /* udelay(2000); */
    /* } */
    /* apa_trim_api(); */
    /* [> audio_dac_analog_open(delay_flag); <] */
    /* [> audio_dac_analog_vol(audio_dac_analog_vol_l, audio_dac_analog_vol_r); <] */
    /* apa_analog_open_api(delay_flag); */
}

void dac_init_api(u32 sr)
{
    if (false == audio_clk_open_check()) {
        return;
    }
    dac_init(sr, 1);
}

void dac_sr_api(u32 sr)
{
    if (false == audio_clk_open_check()) {
        return;
    }
    auout_sr_api(sr);
    /* u32 dac_sr_set(u32 sr) */
    /* apa_sr_set(apa_sr_lookup(sr)); */
    /* dac_analog_init(); */
}
u32 dac_sr_read(void)
{
    /* return apa_sr_read(); */
    if (false == audio_clk_open_check()) {
        return E_AUDIO_CLK;
    }
    return auout_sr_read();
}

void dac_off_api(void)
{
    if (false == audio_clk_open_check()) {
        return;
    }
    /* rdac_analog_close(); */
    /* apa_analog_close(); */
    /* apa_phy_off(); */
    auout_phy_off();
}
/* void dac_sr_api(u32 sr) */
/* { */
/* dac_sr_set(dac_sr_lookup(sr)); */
/* } */

void sound_out_hook(sound_out_obj *psound)
{
#if HAS_MIO_EN
    if (NULL != psound) {
        d_mio_start(psound->mio);
    }
#endif
}

/* #include "common/audio/fill_audac.c" */

u8 stereo_dac_vol(char set, u8 vol_l, u8 vol_r)
{

    if ('+' == set) {
        dac_mge.vol_l++;
        dac_mge.vol_r++;
    } else if ('-' == set) {
        if (0 != dac_mge.vol_l) {
            dac_mge.vol_l--;
        }
        if (0 != dac_mge.vol_r) {
            dac_mge.vol_r--;
        }
    } else if ('r' == set) {
        return dac_mge.vol_l;
    } else if ('R' == set) {
        return dac_mge.vol_r;
    } else {
        /* log_info("vol_l: %d , vol_r: %d", vol_l, vol_r); */
        dac_mge.vol_l = vol_l;
        dac_mge.vol_r = vol_r;
    }
    if (dac_mge.vol_l > MAX_VOL_LEVEL) {
        dac_mge.vol_l = MAX_VOL_LEVEL;
    }
    if (dac_mge.vol_r > MAX_VOL_LEVEL) {
        dac_mge.vol_r = MAX_VOL_LEVEL;
    }
    if (0 == (dac_mge.flag & B_DAC_MUTE)) {
#if DAC_FADE_ENABLE
        if (0 == (dac_mge.flag & B_DAC_FADE_EN))
#endif
        {
            dac_mge.vol_l_phy = vol_tab[dac_mge.vol_l];
            dac_mge.vol_r_phy = vol_tab[dac_mge.vol_r];
        }
        dac_mge.flag &= ~B_DAC_MUTE;
    }
    /* log_info(" dac vol %d, 0x%x\n", dac_mge.vol, dac_mge.vol_phy); */
__dac_vol_end:
    /* apa_phy_vol(dac_mge.vol_l_phy, dac_mge.vol_r_phy); */
    auout_phy_vol(dac_mge.vol_l_phy, dac_mge.vol_r_phy);
    return dac_mge.vol_l;
}
u8 dac_vol(char set, u8 vol)
{
    return stereo_dac_vol(set, vol, vol);
}


bool dac_mute(bool mute)
{
    if (mute) {
        dac_mge.flag |= B_DAC_MUTE;
        dac_mge.vol_l_phy = 0;
        dac_mge.vol_r_phy = 0;
    } else {
        dac_mge.flag &= ~B_DAC_MUTE;
        dac_mge.vol_l_phy = vol_tab[dac_mge.vol_l];
        dac_mge.vol_r_phy = vol_tab[dac_mge.vol_r];
    }
    /* apa_phy_vol(dac_mge.vol_l_phy, dac_mge.vol_r_phy); */
    auout_phy_vol(dac_mge.vol_l_phy, dac_mge.vol_r_phy);
    return true;
}

/*----------------------------------------------------------------------------*/
/**@brief   注册dac缓存函数
   @param   psound_pre	:DAC前级SOUND
			psound_later:注册给DAC的SOUND
			kick		:回调函数
   @return  TRUE:成功   false:失败
   @author  note    bool regist_dac_channel(void *psound_pre, void *psound_later, void *kick)
*/
/*----------------------------------------------------------------------------*/
bool regist_dac_channel(void *psound_pre, void *psound_later, void *kick)
{
    u8 i;
    for (i = 0; i < AUDAC_CHANNEL_TOTAL; i++) {
        if (dac_mge.ch & BIT(i)) {
            continue;
        }
        /* dac_mge.obuf[i] = obuf; */
        dac_mge.kick[i] = kick;
        dac_mge.sound_pre[i] = psound_pre;
        dac_mge.sound_later[i] = psound_later;
        dac_mge.ch |= BIT(i);
        /* log_info("dac_channel :0x%x 0x%x\n", i, dac_mge.ch); */
        return true;
    }
    return false;
}

/*----------------------------------------------------------------------------*/
/**@brief   注销dac缓存函数
   @param   psound	:注册给DAC的SOUND
   @return
   @author  note    bool unregist_dac_channel(void *psound)
*/
/*----------------------------------------------------------------------------*/
bool unregist_dac_channel(void *psound)
{
    u8 i;
    sound_out_obj *ps = psound;

    for (i = 0; i < AUDAC_CHANNEL_TOTAL; i++) {
        if (0 == (dac_mge.ch & BIT(i))) {
            continue;
        }
        if (dac_mge.sound_later[i] == psound) {
            local_irq_disable();
            dac_mge.ch &= ~BIT(i);
            /* dac_mge.obuf[i] = 0; */
            dac_mge.sound_pre[i] = NULL;
            dac_mge.sound_later[i] = NULL;
            dac_mge.kick[i] = NULL;
            ps->enable &= ~B_DEC_OBUF_EN;
            local_irq_enable();
            break;
        }
    }
    return true;
}


AT(.audio_isr_text)
bool dac_cbuff_active(void *sound_hld)
{
    sound_out_obj *psound = sound_hld;
    if (psound->enable & (B_DEC_PAUSE | B_DEC_FIRST)) {
        if (cbuf_get_data_size(psound->p_obuf) >= (cbuf_get_space(psound->p_obuf) / 2)) {
            psound->enable &= ~B_DEC_FIRST;
            return true;
        }
        return false;
    } else {
        return true;
    }
    /* if (psound->enable & B_DEC_PAUSE) { */
    /*     return false; */
    /* } else { */
    /*     return true; */
    /* } */
}
void uac_audio_dac_percent(u8 ch)
{

    u32 src_ibuf = 0;
    u32 src_ibuf_size = 0;
    u32 src_obuf = 0;
    u32 src_obuf_size = 0;
    u32 insample = 0;
    u32 outsample = 0;
    u32 percent = 0;
    if (0 == (dac_mge.ch & BIT(ch))) {
        return;
    }
    if ((dac_mge.sound_later[ch] == NULL) || (dac_mge.sound_pre[ch] == NULL)) {
        return;
    }

    if (B_DEC_FIRST & dac_mge.sound_later[ch]->enable) {	//是否首次开启
        return;
    }
    EFFECT_OBJ *p_effect = dac_mge.sound_pre[ch]->effect;
    sound_in_obj *p_src_si = p_effect->p_si;
    if (NULL == p_src_si) {		//即没有src 或 其他音效
        return;
    }

    SRC_STUCT_API *p_ops = p_src_si->ops;
    /* 1.读ibuf 数据长度 */
    src_ibuf = cbuf_get_data_size(dac_mge.sound_pre[ch]->p_obuf);
    src_ibuf_size = cbuf_get_space(dac_mge.sound_pre[ch]->p_obuf);
    /* 2.前后级采样率	 */
    insample = p_ops->config(
                   p_src_si->p_dbuf,
                   SRC_CMD_GET_INSAMPLE,
                   (void *)NULL
               );
    insample /= 1000;

    outsample = p_ops->config(
                    p_src_si->p_dbuf,
                    SRC_CMD_GET_OUTSAMPLE,
                    (void *)NULL
                );
    outsample /= 1000;


    /* 3.读src_obuf 数据长度*/
    src_obuf = cbuf_get_data_size(dac_mge.sound_later[ch]->p_obuf);
    src_obuf_size = cbuf_get_space(dac_mge.sound_later[ch]->p_obuf);
    /* log_info("%d %d\n",src_ibuf,src_obuf); */
    /* 4. 1和2的百分比*/
    percent = (((src_ibuf * outsample / insample) + src_obuf) * 100) / ((src_ibuf_size * outsample / insample) + src_obuf_size);
    audio_link_sync_accumulate(ch, percent);

}
void audio_dac_sync_once(void)
{
    u8 i;
    EFFECT_OBJ *p_effect = NULL;
    for (i = 0; i < AUDAC_CHANNEL_TOTAL; i++) {
        if (0 == (dac_mge.ch & BIT(i))) {
            continue;
        }
        if (NULL == dac_mge.sound_later[i]) {
            continue;
        }
        if (B_DEC_FIRST & dac_mge.sound_later[i]->enable) {	//是否首次开启
            continue;
        }
        p_effect = dac_mge.sound_later[i]->effect;
        if (p_effect == NULL) {
            log_error("this %d channel no src\n", i);
            continue;
        }
        /* 取同步间隔内百分比平均值,做同步 */
        audio_link_sync_percent(i, p_effect);
        audio_link_sync_reset(i);
    }

}

