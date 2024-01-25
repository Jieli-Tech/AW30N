#ifndef __SBC_ENCODER_H__
#define __SBC_ENCODER_H__

#include "typedef.h"

#define SBC_ENC_OUTPUT_MAX_SIZE    (256)//byte

u32 sbc_encode_api(void *p_file, void *input_func, void *output_func);


#endif

