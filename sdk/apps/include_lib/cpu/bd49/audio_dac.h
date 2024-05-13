#ifndef __AUDIO_DAC_H__
#define __AUDIO_DAC_H__

#include "typedef.h"

typedef enum {
    DAC_SOURCE = 0,
    APA_SOURCE = 1,
    ALINE_SOURCE,
} AUDIO_TYPE;

#include "apa.h"
#include "audac.h"
#include "audio_link/audio_link_api.h"


extern u32 fifo_dac_fill(u8 *buf, u32 len, AUDIO_TYPE type);
u32 audio_dac_clr_pnd(void);
void audio_dac_isr(void);
u32 dac_sr_read(void);



void dac_trim_api(void);
void dac_phy_init(u32 sr_sel);
u32 audac_sr_lookup(u32 sr);
void audac_analog_open_api(u32 delay_flag);
void audac_analog_open(u32 delay_flag);
u32 audac_sr_read(void);
u32 dac_sr_set(u32 sr);
void dac_phy_off(void);
void dac_phy_vol(u16 dac_l, u16 dac_r);
void audac_analog_close(void);
extern u32 fill_dac_fill_phy(u8 *buf, u32 len);

#define AUOUT_USE_APA      	1
#define AUOUT_USE_RDAC      2
#define AUOUT_USE_ALINK     3

#define AUOUT_USE_DAC      AUOUT_USE_APA

#if (AUOUT_USE_APA == AUOUT_USE_DAC)
// apa
#define auout_mode_init     apa_mode_init
#define auout_init(n,m)     apa_init(n,m)
#define auout_sr_api(sr)    apa_sr_api(sr)
#define auout_sr_read       apa_sr_read
#define auout_phy_off       apa_off_api
#define auout_phy_vol(l,r)  apa_phy_vol(l,r)
#define DAC_TRACK_NUMBER    APA_TRACK_NUMBER


#elif (AUOUT_USE_RDAC == AUOUT_USE_DAC)
// audio_dac
#define auout_mode_init     audac_mode_init
#define auout_init(n,m)     audac_init(n,m)
#define auout_sr_api(sr)    audac_sr_api(sr)
#define auout_sr_read       audac_sr_read
#define auout_phy_off       dac_phy_off
#define auout_phy_vol(l,r)  dac_phy_vol(l,r)
#define DAC_TRACK_NUMBER   	AUDAC_TRACK_NUMBER


#elif (AUOUT_USE_ALINK == AUOUT_USE_DAC)
// audio_link
#define auout_mode_init     audio_link_init_api
#define auout_init(n,m)     alink_mode_init(n,m)
#define auout_sr_api(sr)    set_audio_link_sr(sr)
#define auout_sr_read       read_audio_link_sr
#define auout_phy_off       alink_mode_uninit
#define auout_phy_vol(l,r)  alink_phy_vol(l,r)
#define DAC_TRACK_NUMBER   	ALINK_TRACK_NUMBER


#endif



#endif
