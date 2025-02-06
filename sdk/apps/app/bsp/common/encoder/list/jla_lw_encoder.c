
#include "app_modules.h"
#if ENCODER_JLA_LW_EN

#include "circular_buf.h"
#include "encoder_mge.h"
#include "audio_enc_api.h"
#include "jla_lw_codec_ctrl.h"
#include "jla_lw_encoder.h"

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[jla_lw_enc]"
#include "log.h"


static cbuffer_t cbuf_enc_jla_lw_o          AT(.enc_jla_lw_data);
static u32 obuf_ejla_lw_o[1024 / 4]         AT(.enc_jla_lw_data);

enc_obj enc_jla_lw_hdl;
static u32 jla_lw_encode_buff[0x1244 / 4]   AT(.enc_jla_lw_data);   //dbuf
static ENC_OPS jla_lw_enc_ops               AT(.enc_jla_lw_data);
static EN_FILE_IO jla_lw_enc_io             AT(.enc_jla_lw_data);

extern const u8 jla_lw_br_level;

u32 jla_lw_encode_api(void *p_file, void *input_func, void *output_func)
{

    log_info("jla_lw_encode_api\n");

    struct jla_lw_codec_ops *jla_lw_ops;
    jla_lw_ops = get_jla_lw_ops();
    int dbuff_len = jla_lw_ops->need_buf();
    if (dbuff_len > sizeof(jla_lw_encode_buff)) {
        log_error("jla_lw need buf 0x%x \n", dbuff_len);
        return -1;
    }

    if ((jla_lw_br_level < 0) || (jla_lw_br_level > 1)) {
        log_error("current br level %d don't support \n", jla_lw_br_level);
        return -1;
    }
    memset(&obuf_ejla_lw_o[0], 0x00, sizeof(obuf_ejla_lw_o));
    memset(&jla_lw_encode_buff[0], 0x00, sizeof(jla_lw_encode_buff));
    cbuf_init(&cbuf_enc_jla_lw_o, &obuf_ejla_lw_o[0], sizeof(obuf_ejla_lw_o));
    enc_jla_lw_hdl.p_file = p_file;
    enc_jla_lw_hdl.p_ibuf = REC_ADC_CBUF;
    enc_jla_lw_hdl.p_obuf = &cbuf_enc_jla_lw_o;
    enc_jla_lw_hdl.p_dbuf = &jla_lw_encode_buff[0];
    enc_jla_lw_hdl.enc_ops = &jla_lw_enc_ops;

    jla_lw_enc_io.priv        = &enc_jla_lw_hdl;
    jla_lw_enc_io.input_data  = input_func;
    jla_lw_enc_io.output_data = output_func;

    jla_lw_enc_ops.open = jla_lw_encoder_open;
    jla_lw_enc_ops.run = jla_lw_encoder_run;

    enc_jla_lw_hdl.info.sr = read_audio_adc_sr();
    enc_jla_lw_hdl.info.br = enc_jla_lw_hdl.info.sr / 100 / 8 * 18 / 10;
    enc_jla_lw_hdl.info.nch = 1;
    log_info("now sr %d br %d\n", enc_jla_lw_hdl.info.sr, enc_jla_lw_hdl.info.br);

    jla_lw_enc_ops.open((u8 *)jla_lw_encode_buff, &jla_lw_enc_io);

    log_info("JLA_LW enc init succ\n");
    return (u32)&enc_jla_lw_hdl;
}
#endif


