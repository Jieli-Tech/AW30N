#include "audio_link_api.h"
#include "audio_link_sync.h"
#include "audio_adc_api.h"
#include "audio_adc.h"
#include "audio_dac.h"
#include "sound_mge.h"
#include "uac_sync.h"



typedef struct _ALINK_SYNC_CFG {
    uac_sync audio_link_sync[AUDIO_LINK_CHANNEL_TOTAL];
    u32 percent[AUDIO_LINK_CHANNEL_TOTAL];
    u32 percent_cnt[AUDIO_LINK_CHANNEL_TOTAL];
} ALINK_SYNC_CFG;

ALINK_SYNC_CFG alink_sync_mge;

void audio_link_sync_init(u8 ch, s32 sr)
{
    memset(&alink_sync_mge.percent[ch], 0, sizeof(alink_sync_mge.percent[ch]));
    memset(&alink_sync_mge.percent_cnt[ch], 0, sizeof(alink_sync_mge.percent_cnt[ch]));
    uac_sync_init(&alink_sync_mge.audio_link_sync[ch], ALINK_SR_DEFAULT);
}

void audio_link_sync_accumulate(u8 ch, u32 percent)
{
    alink_sync_mge.percent[ch] += percent;
    alink_sync_mge.percent_cnt[ch]++;
}

void audio_link_sync_reset(u8 ch)
{
    alink_sync_mge.percent[ch] = 0;
    alink_sync_mge.percent_cnt[ch] = 0;
}

void audio_link_sync_once(void *priv)
{
#if ((AUDIO_ADC_TYPE != AUIN_USE_ALINK) && (AUOUT_USE_ALINK == AUOUT_USE_DAC))
    audio_dac_sync_once();
#elif ((AUDIO_ADC_TYPE == AUIN_USE_ALINK) && (AUOUT_USE_ALINK != AUOUT_USE_DAC))
    audio_adc_sync_once();
#endif
}

void audio_link_sync_percent(u8 ch, EFFECT_OBJ *e_obj)
{
    if (alink_sync_mge.percent_cnt[ch] == 0) {
        return;
    }
    u32 percent = alink_sync_mge.percent[ch] / alink_sync_mge.percent_cnt[ch];
    /* log_info("2 percent %d %d %d\n",percent,alink_sync_mge.percent[ch], alink_sync_mge.percent_cnt[ch]); */
    uac_inc_sync_one(e_obj, percent, &alink_sync_mge.audio_link_sync[ch]);
}

