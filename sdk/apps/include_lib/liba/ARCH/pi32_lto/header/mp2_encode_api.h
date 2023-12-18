#ifndef _MP2_ENCODE_API_H_
#define _MP2_ENCODE_API_H_

#include "audio_enc_api.h"

//obuf
#define MP2_ENC_OBUF_SIZE           (1024)
//dbuf
#define MONO_MP2_ENC_DBUF_SIZE      (6004)

extern ENC_OPS *get_mp2_ops();

#define get_mp2standard_ops get_mp2_ops

#endif
