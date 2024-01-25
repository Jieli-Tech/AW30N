#include "app_modules.h"

#if defined(DECODER_SBC_EN) && (DECODER_SBC_EN)
#include "circular_buf.h"
#include "typedef.h"
#include "errno-base.h"
#include "if_decoder_ctrl.h"
#include "decoder_api.h"
#include "decoder_mge.h"
#include "audio_dac_api.h"

#include "sbc_api.h"
#include "sbc_stddec_api.h"

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[sbc_api]"
#include "log.h"

#define DEC_SBC_TYPE 0		//决定解码格式0: SBC； 1: MSBC

#define SBC_OBUF_SIZE (DAC_DECODER_BUF_SIZE * 4)
#define SBC_KICK_SIZE (SBC_OBUF_SIZE - (SBC_DEC_OUTPUT_MAX_SIZE * 2))

dec_obj dec_sbc_hld;
cbuffer_t cbuf_sbc				  	 AT(.sbc_dec_data);
u32 obuf_sbc[SBC_OBUF_SIZE / 4]		 AT(.sbc_dec_data);
u32 sbc_decode_buff[0x1354 / 4] 		 AT(.sbc_dec_data);	//受input、output大小影响

struct if_decoder_io sbc_dec_io0 	 AT(.sbc_dec_data);

#define SBC_CAL_BUF ((void *)&sbc_decode_buff[0])		//运行buff

u32 sbc_decode_api(void *strm, void **p_dec, void *p_dp_buf)
{
    log_info("sbc_decode_api");
    dec_data_stream *p_strm = strm;
    u32 buff_len, i = 0;
    decoder_ops_t *ops;
    memset(&dec_sbc_hld, 0, sizeof(dec_obj));
    dec_sbc_hld.type = D_TYPE_SBC;

    ops = get_sbc_stdec_ops();
    buff_len = ops->need_dcbuf_size();
    if (buff_len > sizeof(sbc_decode_buff)) {
        log_error("sbc dbuff : 0x%x 0x%x\n", buff_len, (int)sizeof(sbc_decode_buff));
        return E_SBC_DBUF;
    }

    /******************************************/
    memcpy(&sbc_dec_io0, p_strm->io, sizeof(struct if_decoder_io));
    sbc_dec_io0.priv      = &dec_sbc_hld;

    cbuf_init(&cbuf_sbc, &obuf_sbc[0], sizeof(obuf_sbc));
    /* debug_puts("A\n"); */
    sound_stream_obj *psound_strm = p_strm->strm_source;
    dec_sbc_hld.p_file       = psound_strm;
    dec_sbc_hld.sound.p_obuf = &cbuf_sbc;
    dec_sbc_hld.sound.para = SBC_KICK_SIZE;
    dec_sbc_hld.p_dbuf       = SBC_CAL_BUF;
    dec_sbc_hld.dec_ops      = ops;
    dec_sbc_hld.event_tab    = (u8 *)&sbc_evt[0];
    dec_sbc_hld.sr		 	 = p_strm->sr;

    if (B_DEC_IS_STRM & p_strm->strm_ctl) {
        psound_strm->kick_thr = 80;
    }
    /******************************************/
    ops->open(SBC_CAL_BUF, &sbc_dec_io0, NULL);         //传入io接口，说明如下

    AUDIO_DEC_CH_OPUT_PARA sbcset;
    sbcset.mode = 3;
    sbcset.pL = 8192;
    sbcset.pR = 8192;
    ops->dec_confing(SBC_CAL_BUF, SET_DEC_CH_MODE, &sbcset);

    AUDIO_DECODE_PARA sbco_m;   //设置是否完全输出方式.
    sbco_m.mode = 1;
    ops->dec_confing(SBC_CAL_BUF, SET_DECODE_MODE, &sbco_m);

#if DEC_SBC_TYPE
    ops->dec_confing(SBC_CAL_BUF, SET_FORMAT_MSBC, &sbco_m);	//msbc时需要配置此参数
#endif

    *p_dec = (void *)&dec_sbc_hld;
    return 0;
}

extern const u8 sbc_buf_start[];
extern const u8 sbc_buf_end[];
u32 sbc_buff_api(dec_buf *p_dec_buf)
{
    p_dec_buf->start = (u32)&sbc_buf_start[0];
    p_dec_buf->end   = (u32)&sbc_buf_end[0];
    return 0;
}

u32 set_sbc_Headerstate_api(u32 sr, u32 br, int (*sbc_goon_cb)(void *))
{
    /* log_info("set_sbc_Headerstate_api\n"); */
    return SBC_DEC_INPUT_MAX_SIZE;
}

#endif

