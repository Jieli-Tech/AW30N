
#ifndef __AUDIO_LINK_API_H__
#define __AUDIO_LINK_API_H__

#include "typedef.h"
#include "audio_link.h"
#include "sound_mge/sound_mge.h"

u32 audio_link_init_api(void);
u32 audio_link_uninit_api(void);

u32 audio_link_init_channel_api(ALINK_CH_CFG *alink_data_parm);
u32 audio_link_uninit_channel_api(ALINK_CH_CFG *alink_data_parm);

bool regist_audio_link_channel(void *pre_sound, void *later_sound, void *kick);
bool unregist_audio_link_channel(void *psound);

int audio_link_en_api(enum ALINK_MODULE_EN en);

void fill_audio_link_rx(u8 ch, s16 *buf, u32 len);
u32 fill_audio_link_tx(u8 ch, u8 *buf, u32 len);
void sync_audio_link_percent(u8 ch);


void set_audio_link_sr(u32 sr);
u16 audio_link_set_sr_api(u32 rate);
void alink_phy_vol(u16 dac_l, u16 dac_r);
void alink_mode_init();
void alink_mode_uninit();
u16 read_audio_link_sr();

void auadc_open_alink(void);
void auadc_close_alink(void);
u32 audio_link_phy_init(void *p_adc, u32 sr, u32 con, u32 throw);

#endif
