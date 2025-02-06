#ifndef __ENCODER_STREAM_H__
#define __ENCODER_STREAM_H__

#include "typedef.h"
#include "encoder_mge.h"
// #include "audio_enc_api.h"
// #include "sound_mge.h"
// #include "audio_adc.h"

#include "audio_codec_format.h"
enc_obj *audio2rf_encoder_io(u32(*enc_fun)(void *, void *, void *),  void *p_queue_ops, void *ops);
u32 audio2rf_encoder_start(enc_obj *obj);


#endif
