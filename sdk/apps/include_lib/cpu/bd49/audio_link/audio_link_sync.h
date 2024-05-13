
#ifndef __AUDIO_LINK_SYNC_H__
#define __AUDIO_LINK_SYNC_H__

#include "typedef.h"
#include "sound_mge/sound_mge.h"



void audio_link_sync_accumulate(u8 ch, u32 percent);
void audio_link_sync_reset(u8 ch);
void audio_link_sync_percent(u8 ch, EFFECT_OBJ *e_obj);
void audio_link_sync_init(u8 ch, s32 sr);
void audio_link_sync_once(void *priv);




#endif
