
#include "app_modules.h"

#if ENCODER_MP3_EN

#include "encoder_mge.h"
#include "cpu.h"
#include "config.h"
#include "typedef.h"
#include "hwi.h"
#include "mp2_encode_api.h"

#include "circular_buf.h"


#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[normal]"
#include "log.h"

/* ---------------------------------------------------------------------------- */
/* |标准MP2编码 码率（BR）/采样率（SR）对应关系                               | */
/* ---------------------------------------------------------------------------- */
/* |采样率(SR)  | 8000  12000   16000   22050   24000   | 32000 44100   48000 | */
/* ---------------------------------------------------------------------------- */
/* |码率(BR)    | 8kbps                                 | 32kbps              | */
/* |            | 16kbps                                | 48kbps              | */
/* |            | 24kbps                                | 56kbps              | */
/* |            | 32kbps                                | 64kbps              | */
/* |            | 40kbps                                | 80kbps              | */
/* |            | 48kbps                                | 96kbps              | */
/* |            | 56kbps                                | 112kbps             | */
/* |            | 64kbps                                | 128kbps             | */
/* |            | 80kbps                                | 160kbps             | */
/* |            | 96kbps                                | 192kbps             | */
/* |            | 112kbps                               | 224kbps             | */
/* |            | 128kbps                               | 256kbps             | */
/* |            | 144kbps                               | 320kbps             | */
/* |            | 160kbps                               | 384kbps             | */
/* ---------------------------------------------------------------------------- */



static cbuffer_t cbuf_emp3_o AT(.enc_mp3_data);
static u32 obuf_emp3_o[MP2_ENC_OBUF_SIZE / 4] AT(.enc_mp3_data) ;

/* static u32 mp3_encode_buff[1501] AT(.enc_mp3_data) ; */

static enc_obj enc_mp3_hdl;

#if ENCODER_MP3_STEREO
const int mp2_encode_channel = 2;
static u32 mp3_encode_buff[STEREO_MP2_ENC_DBUF_SIZE / 4] AT(.enc_mp3_data) ;
#else
const int mp2_encode_channel = 1;
static u32 mp3_encode_buff[MONO_MP2_ENC_DBUF_SIZE / 4] AT(.enc_mp3_data) ;
#endif

static EN_FILE_IO mp3_enc_io AT(.enc_mp3_data);
/* static const EN_FILE_IO mp3_enc_io = { */
/*     &enc_mp3_hdl,      //input跟output函数的第一个参数，解码器不做处理，直接回传，可以为NULL */
/*     enc_input, */
/*     enc_output, */
/* }; */

u32 mp3_encode_api(void *p_file, void *input_func, void *output_func)
{
    u32 buff_len, i;
    ENC_OPS *ops;
    log_info("mp3_encode_api\n");
    ops = get_mp2standard_ops();
    buff_len = ops->need_buf();
    if (buff_len > sizeof(mp3_encode_buff)) {
        log_info("buff_len : %d", buff_len);
        log_info("mp3_encode_buff len : %d", sizeof(mp3_encode_buff));
        return 0;
    }

    memset(&enc_mp3_hdl, 0, sizeof(enc_obj));
    memset(&obuf_emp3_o[0], 0x00, sizeof(obuf_emp3_o));
    memset(&mp3_encode_buff[0], 0x00, sizeof(mp3_encode_buff));
    /******************************************/
    cbuf_init(&cbuf_emp3_o, &obuf_emp3_o[0], sizeof(obuf_emp3_o));
    log_info("A\n");
    // log_info("B\n");
    mp3_enc_io.priv        = &enc_mp3_hdl;
    mp3_enc_io.input_data  = input_func;
    mp3_enc_io.output_data = output_func;
    enc_mp3_hdl.p_file = p_file;
    enc_mp3_hdl.p_ibuf = REC_ADC_CBUF;//adc_hdl.p_adc_cbuf;//&cbuf_emp3_i;
    enc_mp3_hdl.p_obuf = &cbuf_emp3_o;
    enc_mp3_hdl.p_dbuf = &mp3_encode_buff[0];
    enc_mp3_hdl.enc_ops = ops;
    enc_mp3_hdl.info.sr = read_audio_adc_sr();
    /*br取值表：{8,16,24,32,40,48,56,64,80,96,112,128,144,160}*/
    enc_mp3_hdl.info.br = 80;
    enc_mp3_hdl.info.nch = mp2_encode_channel;
    enc_mp3_hdl.info.nch = 1;

    log_info("encoder sr %d\n", enc_mp3_hdl.info.sr);
    /******************************************/
    ops->open((void *)&mp3_encode_buff[0], (void *)&mp3_enc_io);  //传入io接口，说明如下
    ops->set_info((void *)&mp3_encode_buff[0], &enc_mp3_hdl.info);
    ops->init((void *)&mp3_encode_buff[0]);
    /* enc_mp3_hdl.enable = B_ENC_ENABLE; */
    //debug_u32hex(enc_mp3_hdl.enable);
    return (u32)&enc_mp3_hdl;
}

#endif
