#ifndef __SPEEX_ENCODER_H__
#define __SPEEX_ENCODER_H__

#include "typedef.h"

#define SPEEX_ENC_OUTPUT_MAX_SIZE     (640)//byte

u32 speex_encode_api(void *p_file, void *input_func, void *output_func);


#endif

