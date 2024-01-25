
#include "app_modules.h"

#if defined(DECODER_SPEEX_EN) && (DECODER_SPEEX_EN)
#include "circular_buf.h"
#include "typedef.h"
#include "errno-base.h"
#include "if_decoder_ctrl.h"
#include "decoder_api.h"
#include "decoder_mge.h"
#include "audio_dac_api.h"

#include "speex_api.h"
#include "speex_encode_api.h"

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[speex_api]"
#include "log.h"

#define SPEEX_OBUF_SIZE (DAC_DECODER_BUF_SIZE * 4)
#define SPEEX_KICK_SIZE (SPEEX_OBUF_SIZE - (SPEEX_DEC_OUTPUT_MAX_SIZE * 2))
const  int  SPEEX_DECOUT_2CH_EN = 0;//0代表单声道输出, 1代表双声道输出

dec_obj dec_speex_hld;
cbuffer_t cbuf_speex					 AT(.speex_dec_data);
u32 obuf_speex[SPEEX_OBUF_SIZE / 4]		 AT(.speex_dec_data);
u32 speex_decode_buff[0x17cc / 4] 		 AT(.speex_dec_data);

struct if_decoder_io speex_dec_io0 	 AT(.speex_dec_data);

#define SPEEX_CAL_BUF ((void *)&speex_decode_buff[0])		//运行buff

u32 speex_decode_api(void *strm, void **p_dec, void *p_dp_buf)
{
    log_info("speex_decode_api");

    dec_data_stream *p_strm = strm;
    u32 buff_len = 0;
    decoder_ops_t *ops;
    ops = get_speex_ops();
    memset(&dec_speex_hld, 0, sizeof(dec_obj));
    dec_speex_hld.type = D_TYPE_SPEEX;

    buff_len = ops->need_dcbuf_size();
    if (buff_len > sizeof(speex_decode_buff)) {
        log_info("speex dbuff : 0x%x 0x%x\n", buff_len, (int)sizeof(speex_decode_buff));
        return E_SPEEX_DBUF;
    }

    /******************************************/
    memcpy(&speex_dec_io0, p_strm->io, sizeof(struct if_decoder_io));
    speex_dec_io0.priv      = &dec_speex_hld;

    cbuf_init(&cbuf_speex, &obuf_speex[0], sizeof(obuf_speex));
    sound_stream_obj *psound_strm = p_strm->strm_source;
    dec_speex_hld.p_file       = psound_strm;
    dec_speex_hld.sound.p_obuf = &cbuf_speex;
    dec_speex_hld.sound.para = SPEEX_KICK_SIZE;
    dec_speex_hld.p_dbuf       = SPEEX_CAL_BUF;
    dec_speex_hld.dec_ops      = ops;
    dec_speex_hld.event_tab    = (u8 *)&speex_evt[0];
    dec_speex_hld.sr		 	 = p_strm->sr;

    if (B_DEC_IS_STRM & p_strm->strm_ctl) {
        psound_strm->kick_thr = 42;
    }
    /******************************************/
    ops->open(SPEEX_CAL_BUF, &speex_dec_io0, NULL);         //传入io接口，说明如下

    AUDIO_DECODE_PARA decode_mode;
    decode_mode.mode = 1;
    ops->dec_confing(SPEEX_CAL_BUF, SET_DECODE_MODE, &decode_mode);		//配置解码模式

    ops->dec_confing(SPEEX_CAL_BUF, CMD_SET_SAMPLE, &dec_speex_hld);		//配置采样率

    ops->format_check(SPEEX_CAL_BUF);

    *p_dec = (void *)&dec_speex_hld;
    return 0;
}

extern const u8 speex_buf_start[];
extern const u8 speex_buf_end[];
u32 speex_buff_api(dec_buf *p_dec_buf)
{
    p_dec_buf->start = (u32)&speex_buf_start[0];
    p_dec_buf->end   = (u32)&speex_buf_end[0];
    return 0;
}
u32 set_speex_Headerstate_api(u32 sr, u32 br, int (*opus_goon_cb)(void *))
{
    decoder_ops_t *ops;
    ops = get_speex_ops();
    SR_CONTEXT speex_sr;
    speex_sr.sr = sr;	//8k / 16k
    log_info("set_speex_Headerstate_api sr %d \n", sr);
    ops->dec_confing(SPEEX_CAL_BUF, CMD_SET_SAMPLE, &speex_sr);		//配置采样率
    return SPEEX_DEC_INPUT_MAX_SIZE;
}

#endif

