#ifndef _SPEEX_ENCODE_API_H_
#define _SPEEX_ENCODE_API_H_

#include "if_decoder_ctrl.h"

#ifndef u16
#define  u32  unsigned int
#define  u16  unsigned short
#define  s16  short
#define  u8  unsigned char
#endif

#define  ENCODE_FAST_FLAG  0x80

typedef struct _SPEEX_EN_FILE_IO_ {
    void *priv;
    u16(*input_data)(void *priv, s16 *buf, u8 channel, u16 len);           //short
    u16(*output_data)(void *priv, u8 *buf, u16 len);        //bytes
} SPEEX_EN_FILE_IO;


typedef struct __SPEEX_ENC_OPS {
    u32(*need_buf)(u16 samplerate);
    u32(*open)(u8 *ptr, SPEEX_EN_FILE_IO *audioIO, u8 quality, u16 sample_rate);
    u32(*run)(u8 *ptr);
} speex_enc_ops;


typedef struct __SPEEX_DEC_OPS {
    u32(*need_buf)();
    u32(*open)(u32 *ptr, int sr);
    u32(*run)(u32 *ptr, char *frame_data, short *frame_out);
} speex_dec_ops;


typedef struct  _SR_CONTEXT_ {
    int sr;
} SR_CONTEXT;

extern speex_enc_ops *get_speex_enc_obj();         //����
extern audio_decoder_ops *get_speex_ops();         //解码
extern speex_dec_ops *get_speex_dec_obj();         //����

#endif


#if 0

unsigned short e_input_data(void *priv, short *buf, u8 chn, unsigned short len)
{
    //get data form adc
    return (fread(buf, sizeof(short), len, fin));
}

void e_output_data(void *priv, unsigned char *buf, unsigned short len)
{
    unsigned char  tv[4];
    tv[0] = len;
    tv[1] = 0x53;
    tv[2] = 0x50;
    tv[3] = 0x58;

    fwrite(tv, 1, 4, fbits);          //header for   duerOS
    fwrite(buf, 1, len, fbits);       //speex  data
}

SPEEX_EN_FILE_IO speex_en_io = {
    NULL,
    e_input_data,
    e_output_data
};



//call demo
{
    unsigned char *enc_buf;
    speex_enc_ops *test_ops = get_speex_enc_obj();
    enc_buf = malloc(test_ops->need_buf(8000));
    if (test_ops->open(enc_buf, &speex_en_io, 8, 8000) == 0) {
        // init failed, sample_rate error
        return;
    }


    while (recoding) {
        test_ops->run(enc_buf);
    }




}



#endif