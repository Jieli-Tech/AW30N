#ifndef ADPCM_STREAM_ENC_API_H
#define ADPCM_STREAM_ENC_API_H

#include "audio_enc_api.h"
extern const int ima_enc_stream_max_input;   //npoint
extern const int ima_enc_stream_max_output;  //byte

extern ENC_OPS *get_ima_adpcm_stream_code_ops();
extern int need_ima_stream_encbk_buf();		//断点信息大小
extern u32 get_ima_stream_encbp_info(void *work_buf); //返回断点信息地址

#endif
