#ifndef __OPUS_ENCODER_H__
#define __OPUS_ENCODER_H__
#include "typedef.h"

#define OPUS_ENC_INPUT_MAX_SIZE     (320 * 2)
#define OPUS_ENC_OUTPUT_MAX_SIZE    (528)

#define QY_BR_CONFIG_BIT_OFFSET     (0)
#define QY_BR_CONFIG_16KBPS         (0)
#define QY_BR_CONFIG_32KBPS         (1)
#define QY_BR_CONFIG_64KBPS         (2)

#define QY_COMPLEXITY_BIT_OFFSET    (4)
#define QY_COMPLEXITY_HIGH          (0)
#define QY_COMPLEXITY_LOW           (1)

#define QY_FORMAT_MODE_BIT_OFFSET   (6)
#define QY_FORMAT_MODE_BAIDU        (0)
#define QY_FORMAT_MODE_KUGOU        (1)
#define QY_FORMAT_MODE_OGG          (2)

// void opus_enc_output_cb_sel(u32 opus_enc_output_func(void *, u8 *, u16));
u32 opus_encode_api(void *p_file, void *input_func, void *output_func);
#endif
