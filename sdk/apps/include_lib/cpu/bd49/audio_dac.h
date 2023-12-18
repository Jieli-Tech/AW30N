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

#define AUOUT_USE_RDAC      0

#if (0 == AUOUT_USE_RDAC)
//apa
#define auout_mode_init     apa_mode_init
#define auout_init(n,m)     apa_init(n,m)
#define auout_sr_api(sr)    apa_sr_api(sr)
#define auout_sr_read       apa_sr_read
#define auout_phy_off       apa_off_api
#define auout_phy_vol(l,r)  apa_phy_vol(l,r)

#define DAC_TRACK_NUMBER    APA_TRACK_NUMBER
#else
// audio_dac
#define auout_mode_init     audac_mode_init
#define auout_init(n,m)     audac_init(n,m)
#define auout_sr_api(sr)    audac_sr_api(sr)
#define auout_sr_read       audac_sr_read
#define auout_phy_off       dac_phy_off
#define auout_phy_vol(l,r)  dac_phy_vol(l,r)

#define DAC_TRACK_NUMBER   AUDAC_TRACK_NUMBER
#endif



#endif
