#ifndef __AUDIO_CODEC_FORMAT_H__
#define __AUDIO_CODEC_FORMAT_H__

#include "typedef.h"

typedef enum {
    FORMAT_UMP3 = 0,
    FORMAT_A = 1,
    FORMAT_MP3_ST = 2,
    FORMAT_OPUS = 3,
    FORMAT_IMA = 4,
    FORMAT_SBC = 5,
    FORMAT_SPEEX = 6,
    FORMAT_JLA_LW = 7,
} AUDIO_FORMAT;

u32 select_codec(AUDIO_FORMAT enc_type);
#endif
