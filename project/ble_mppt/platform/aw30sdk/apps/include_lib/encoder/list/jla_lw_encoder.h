#ifndef __JLA_LW_ENCODER_H__
#define __JLA_LW_ENCODER_H__

#include "typedef.h"
#include "audio_enc_api.h"
u32 jla_lw_encode_api(void *p_file, void *input_func, void *output_func);


void jla_lw_encoder_open(u8 *ptr, EN_FILE_IO *audioIO);
u32 jla_lw_encoder_run(u8 *ptr);

#endif


