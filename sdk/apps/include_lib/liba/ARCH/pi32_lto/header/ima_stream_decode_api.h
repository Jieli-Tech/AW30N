#ifndef IMA_DECODE_API_H
#define IMA_DECODE_API_H

#include "if_decoder_ctrl.h"

#define CMD_SET_GOON_CALLBACK 0x95
#define CMD_SET_PRE_IDX		  0x96

extern const int ima_dec_stream_max_input;    //byte
extern const int ima_dec_stream_max_output;   //npoint

typedef struct _Goon_DEC_CallBack_ {
    void *priv;
    int (*callback)(void *priv);
} Goon_DEC_CallBack;

typedef struct _ima_dec_stream_pre_idx {
    short valpred;
    char index;
} ima_dec_stream_pre_idx;

extern audio_decoder_ops *get_ima_adpcm_stream_ops();

#endif
