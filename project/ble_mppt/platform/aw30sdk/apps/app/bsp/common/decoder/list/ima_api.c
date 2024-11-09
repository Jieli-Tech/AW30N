#include "app_modules.h"

#if defined(DECODER_IMA_EN) && (DECODER_IMA_EN)
#include "circular_buf.h"
#include "typedef.h"
#include "errno-base.h"
#include "if_decoder_ctrl.h"
#include "decoder_api.h"
#include "decoder_mge.h"
#include "audio_dac_api.h"

#include "ima_api.h"
#include "ima_stream_decode_api.h"

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[ima_api]"
#include "log.h"

#define IMA_OBUF_SIZE (DAC_DECODER_BUF_SIZE * 4)
#define IMA_KICK_SIZE (IMA_OBUF_SIZE - (IMA_DEC_OUTPUT_MAX_SIZE * 2))

dec_obj dec_ima_hld;
cbuffer_t cbuf_ima				  	 AT(.ima_dec_data);
u32 obuf_ima[IMA_OBUF_SIZE / 4]		 AT(.ima_dec_data);
u32 ima_decode_buff[0x21c / 4] 		 AT(.ima_dec_data);	//受input、output大小影响
static u32 ima_dp_buff[24 / 4]		 AT(.ima_dec_data);

struct if_decoder_io ima_dec_io0 	 AT(.ima_dec_data);

#define IMA_CAL_BUF ((void *)&ima_decode_buff[0])		//运行buff

const int ima_dec_stream_max_input = 80;    //byte
const int ima_dec_stream_max_output = 160;   //样点数

u32 ima_decode_api(void *strm, void **p_dec, void *p_dp_buf)
{
    log_info("ima_decode_api");
    dec_data_stream *p_strm = strm;
    u32 buff_len, i = 0;
    decoder_ops_t *ops;
    memset(&dec_ima_hld, 0, sizeof(dec_obj));
    dec_ima_hld.type = D_TYPE_IMA;

    ops = get_ima_adpcm_stream_ops();
    buff_len = ops->need_dcbuf_size();
    if (buff_len > sizeof(ima_decode_buff)) {
        log_error("ima dbuff : 0x%x 0x%x\n", buff_len, (int)sizeof(ima_decode_buff));
        return E_IMA_DBUF;
    }

    /******************************************/
    memcpy(&ima_dec_io0, p_strm->io, sizeof(struct if_decoder_io));
    ima_dec_io0.priv      = &dec_ima_hld;

    cbuf_init(&cbuf_ima, &obuf_ima[0], sizeof(obuf_ima));
    /* debug_puts("A\n"); */
    sound_stream_obj *psound_strm = p_strm->strm_source;
    dec_ima_hld.p_file       = psound_strm;
    dec_ima_hld.sound.p_obuf = &cbuf_ima;
    dec_ima_hld.sound.para = IMA_KICK_SIZE;
    dec_ima_hld.p_dbuf       = IMA_CAL_BUF;
    dec_ima_hld.dec_ops      = ops;
    dec_ima_hld.event_tab    = (u8 *)&ima_evt[0];
    dec_ima_hld.p_dp_buf     = &ima_dp_buff[0];// p_dp_buf;
    dec_ima_hld.sr           = p_strm->sr;

    if (B_DEC_IS_STRM & p_strm->strm_ctl) {
        psound_strm->kick_thr = 80;
    }

    /******************************************/
    ops->open(IMA_CAL_BUF, &ima_dec_io0, NULL);         //传入io接口，说明如下
    memcpy(&ima_dp_buff[0], (void *)ops->get_bp_inf(IMA_CAL_BUF), sizeof(ima_dp_buff));
    *p_dec = (void *)&dec_ima_hld;
    return 0;
}

extern const u8 ima_buf_start[];
extern const u8 ima_buf_end[];
u32 ima_buff_api(dec_buf *p_dec_buf)
{
    p_dec_buf->start = (u32)&ima_buf_start[0];
    p_dec_buf->end   = (u32)&ima_buf_end[0];
    return 0;
}
void set_ima_input_goon_cb(int (*ima_goon_cb)(void *))
{
    log_info("set_ima_input_goon_cb\n");
    GoOn_DEC_CallBack gocio = {0};
    gocio.priv = &dec_ima_hld;
    gocio.callback = ima_goon_cb;
    decoder_ops_t *ops = get_ima_adpcm_stream_ops();
    ops->dec_confing(IMA_CAL_BUF, CMD_SET_GOON_CALLBACK, &gocio);
}

u32 set_ima_Headerstate_api(u32 sr, u32 br, int (*ima_goon_cb)(void *))
{
    log_info("set_ima_Headerstate_api\n");
    decoder_ops_t *ops = get_ima_adpcm_stream_ops();
    set_ima_input_goon_cb(ima_goon_cb);
    return IMA_DEC_INPUT_MAX_SIZE;
}

#endif

