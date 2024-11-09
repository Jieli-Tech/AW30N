#ifndef ADPCM_STREAM_ENC_API_H
#define ADPCM_STREAM_ENC_API_H

#include "audio_enc_api.h"

extern const int ima_enc_stream_header;

extern const int ima_enc_stream_max_input;   //点数大小
extern const int ima_enc_stream_max_output;  //字节大小

extern ENC_OPS *get_ima_adpcm_stream_code_ops();

#endif
