
#include "app_modules.h"

#if defined(DECODER_JLA_LW_EN) && (DECODER_JLA_LW_EN)
#include "circular_buf.h"
#include "typedef.h"
#include "errno-base.h"
#include "if_decoder_ctrl.h"
#include "decoder_api.h"
#include "decoder_mge.h"
#include "audio_dac_api.h"
#include "jla_lw_codec_ctrl.h"
#include "jla_lw_api.h"

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[jla_lw_api]"
#include "log.h"

#define JLA_LW_DEC_OUTPUT_MAX_SIZE  (320 * 2)
#define JLA_LW_OBUF_SIZE (DAC_DECODER_BUF_SIZE * 4)
#define JLA_LW_KICK_SIZE (JLA_LW_OBUF_SIZE - (JLA_LW_DEC_OUTPUT_MAX_SIZE * 2))

/* PLC 和 淡入淡出参数 */
const int JLA_LW_PLC_EN = 0;    //0.
const int JLA_LW_PLC_FADE_OUT_START_POINT = 1200;  // 20; // 1200;
const int JLA_LW_PLC_FADE_OUT_POINTS = 120 * 2;
const int JLA_LW_PLC_FADE_IN_POINTS = 120 * 2;


cbuffer_t cbuf_dec_jla_lw_st                AT(.jla_lw_dec_data);
u16 obuf_dec_jla_lw[JLA_LW_OBUF_SIZE / 2]   AT(.jla_lw_dec_data);

static u32 jla_lw_decode_buff[0x1244 / 4]   AT(.jla_lw_dec_data);   //dbuf,运算buff

dec_obj dec_jla_lw_hld;
struct if_decoder_io jla_lw_dec_io0         AT(.jla_lw_dec_data);
static decoder_ops_t jla_lw_ops;



extern const u8 jla_lw_buf_start[];
extern const u8 jla_lw_buf_end[];
u32 jla_lw_buff_api(dec_buf *p_dec_buf)
{
    p_dec_buf->start = (u32)&jla_lw_buf_start[0];
    p_dec_buf->end   = (u32)&jla_lw_buf_end[0];
    return 0;
}

u32 jla_lw_decode_api(void *strm, void **p_dec, void *p_dp_buf)
{
    log_info("jla_lw_decode_api\n");

    dec_data_stream *p_strm = strm;
    decoder_ops_t *ops;
    ops = &jla_lw_ops;

    memset(&dec_jla_lw_hld, 0, sizeof(dec_obj));
    cbuf_init(&cbuf_dec_jla_lw_st, &obuf_dec_jla_lw[0], sizeof(obuf_dec_jla_lw));
    dec_jla_lw_hld.type = D_TYPE_JLA_LW;
    sound_stream_obj *psound_strm = p_strm->strm_source;
    dec_jla_lw_hld.p_file       = (void *)psound_strm;
    dec_jla_lw_hld.sound.p_obuf = &cbuf_dec_jla_lw_st;
    dec_jla_lw_hld.sound.para   = JLA_LW_KICK_SIZE;
    dec_jla_lw_hld.p_dbuf       = (void *)jla_lw_decode_buff;
    dec_jla_lw_hld.dec_ops      = ops;
    dec_jla_lw_hld.event_tab    = (u8 *)&jla_lw_evt[0];
    dec_jla_lw_hld.p_dp_buf     = p_dp_buf;

    struct jla_lw_codec_ops *jla_lw_ops;
    jla_lw_ops = get_jla_lw_ops();
    int dbuff_len = jla_lw_ops->need_buf();
    if (dbuff_len > sizeof(jla_lw_decode_buff)) {
        log_error("jla_lw need buf 0x%x \n", dbuff_len);
        return -1;
    }

    memcpy(&jla_lw_dec_io0, p_strm->io, sizeof(struct if_decoder_io));
    jla_lw_dec_io0.priv      = &dec_jla_lw_hld;

    ops->open = jla_lw_decoder_open;
    ops->run = jla_lw_decoder_run;

    dec_jla_lw_hld.sr = p_strm->sr;
    dec_jla_lw_hld.br = p_strm->br;

    u32 ret = ops->open(jla_lw_decode_buff, &jla_lw_dec_io0, NULL);
    if (ret == -1) {
        log_error("jla_lw open fail\n");
        return -1;
    } else {
        log_info("jla_lw open succ\n");
    }

    *p_dec = (void *)&dec_jla_lw_hld;
    return 0;
}







#endif





