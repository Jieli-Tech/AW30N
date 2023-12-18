#include "config.h"
#include "app_modules.h"
#include "typedef.h"
#include "cpu.h"

#include "opus_api.h"
#include "opus_decode_api.h"
#include "decoder_api.h"
#include "audio_dac_api.h"
#include "decoder_msg_tab.h"
#include "app_config.h"
#include "circular_buf.h"
#include "errno-base.h"
#include "msg.h"
#include "vfs.h"

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[opus_dec]"
#include "log.h"

#if defined(DECODER_OPUS_EN) && (DECODER_OPUS_EN)

#define OPUS_DEC_OBUF_SIZE (DAC_DECODER_BUF_SIZE * 4)
#define OPUS_DEC_KICK_SIZE (OPUS_DEC_OBUF_SIZE - (OPUS_DEC_OUTPUT_MAX_SIZE * 2))

dec_obj dec_opus_hld;
cbuffer_t cbuf_opus                     AT(.opus_dec_data);
u16 obuf_opus[OPUS_DEC_OBUF_SIZE / 2]   AT(.opus_dec_data);
u32 opus_decode_buff[0x10d0 / 4]        AT(.opus_dec_data);
#define OPUS_CAL_BUF ((void *)&opus_decode_buff[0])

/* static u32(*opus_mp_input_cb)(void *, u32, void *, int, u8) = NULL; */
/* static int opus_mp_input(void *priv, u32 addr, void *buf, int len, u8 type) */
/* { */
/*     static u32 cnt = 0; */
/*     printf("in  %d %d\n", cnt++, len); */
/*     if (opus_mp_input_cb) { */
/*         return opus_mp_input_cb(priv, addr, buf, len, type); */
/*     } else { */
/*         return mp_input(priv, addr, buf, len, type); */
/*     } */
/* } */
/* void opus_mp_input_cb_sel(u32 opus_mp_input_func(void *, u32, void *, int, u8)) */
/* { */
/*     local_irq_disable(); */
/*     opus_mp_input_cb = opus_mp_input_func; */
/*     local_irq_enable(); */
/* } */
struct if_decoder_io opus_dec_io0 AT(.opus_dec_data);
/* const struct if_decoder_io opus_dec_io0 = { */
/*     &dec_opus_hld,      //input跟output函数的第一个参数，解码器不做处理，直接回传，可以为NULL */
/*     opus_mp_input, */
/*     0, */
/*     mp_output, */
/*     decoder_get_flen, */
/*     0 */
/* }; */

u32 opus_decode_api(void *strm, void **p_dec, void *p_dp_buf)
{
    dec_data_stream *p_strm = strm;
    u32 buff_len, i;
    /* void *name; */
    /* char name[VFS_FILE_NAME_LEN] = {0}; */
    decoder_ops_t *ops;
    log_info("opus_decode_api\n");
    memset(&dec_opus_hld, 0, sizeof(dec_obj));
    memset(&opus_decode_buff, 0, sizeof(opus_decode_buff));

    dec_opus_hld.type = D_TYPE_OPUS;
    ops = get_opusdec_ops();
    buff_len = ops->need_dcbuf_size();
    log_info("opus file dbuff : 0x%x 0x%lx\n", buff_len, sizeof(opus_decode_buff));
    if (buff_len > sizeof(opus_decode_buff)) {
        /* log_info("opus file dbuff : 0x%x 0x%lx\n", buff_len, sizeof(opus_decode_buff)); */
        return E_OPUS_DBUF;
    }
    /******************************************/
    memcpy(&opus_dec_io0, p_strm->io, sizeof(struct if_decoder_io));
    opus_dec_io0.priv      = &dec_opus_hld;

    cbuf_init(&cbuf_opus, &obuf_opus[0], sizeof(obuf_opus));
    sound_stream_obj *psound_strm = p_strm->strm_source;
    dec_opus_hld.p_file       = psound_strm;
    dec_opus_hld.sound.p_obuf = &cbuf_opus;
    dec_opus_hld.sound.para   = OPUS_DEC_KICK_SIZE;
    dec_opus_hld.p_dbuf       = OPUS_CAL_BUF;
    dec_opus_hld.dec_ops      = ops;
    dec_opus_hld.event_tab    = (u8 *)&opus_evt[0];
    dec_opus_hld.p_dp_buf     = p_dp_buf;
    //dac reg
    // dec_opus_hld.dac.obuf = &cbuf_opus;
    // dec_opus_hld.dac.vol = 255;
    // dec_opus_hld.dac.index = reg_channel2dac(&dec_opus_hld.dac);
    /******************************************/

    /* name = vfs_file_name(p_file); */
    log_info(" -opus open\n");
    ops->open(OPUS_CAL_BUF, &opus_dec_io0, p_dp_buf);         //传入io接口，说明如下
    log_info(" -opus open over\n");

    /* int file_len = vfs_file_name(p_file, (void *)g_file_sname, sizeof(g_file_sname)); */
    /* if (B_STRM_NO_CHECK & p_strm->strm_ctl) { */
    /* } */
    if (B_DEC_IS_STRM & p_strm->strm_ctl) {
        psound_strm->kick_thr = (p_strm->br / 16) * 40;
    }

    if (ops->format_check(OPUS_CAL_BUF)) {                  //格式检查
        log_info(" opus format err : %s\n", g_file_sname);
        return E_OPUS_FORMAT;
    }

    AUDIO_DECODE_PARA modevalue = {0};
    modevalue.mode = 1;          //output是否判断返回值
    ops->dec_confing(OPUS_CAL_BUF, SET_DECODE_MODE, &modevalue);

    BR_CONTEXT br_obj = {0};
    br_obj.br_index = 0;
    ops->dec_confing(OPUS_CAL_BUF, SET_DEC_BR, &br_obj);
    /* regist_dac_channel(&dec_opus_hld.sound, kick_decoder);//注册到DAC; */
    /* i = ops->get_dec_inf(OPUS_CAL_BUF)->sr;                //获取采样率 */
    /* dec_opus_hld.sr = i; */
    /* log_info("file sr : %d\n", i); */
    *p_dec = (void *)&dec_opus_hld;
    return 0;
    /* dec_opus_hld.enable = B_DEC_ENABLE | B_DEC_KICK; */
    /* debug_u32hex(dec_opus_hld.enable); */
    /* kick_decoder(); */
    /* return 0; */
}

u32 opus_decode_init(void *strm, void **p_dec, void *p_dp_buf, void *input_func, void *output_func)
{
    return opus_decode_api(strm, p_dec, p_dp_buf);
}

extern const u8 opus_buf_start[];
extern const u8 opus_buf_end[];
u32 opus_buff_api(dec_buf *p_dec_buf)
{
    p_dec_buf->start = (u32)&opus_buf_start[0];
    p_dec_buf->end   = (u32)&opus_buf_end[0];
    return 0;
}

void set_opus_input_goon_cb(int (*opus_goon_cb)(void *))
{
    GoOn_DEC_CallBack gocio = {0};
    gocio.priv = NULL;
    gocio.callback = opus_goon_cb;
    decoder_ops_t *ops = get_opusdec_ops();
    ops->dec_confing(OPUS_CAL_BUF, CMD_SET_GOON_CALLBACK, &gocio);
}

u32 set_opus_Headerstate_api(u32 sr, u32 br, int (*opus_goon_cb)(void *))
{
    BR_CONTEXT br_obj = {0};
    if (br == 16) {
        br_obj.br_index = 0;
    } else if (br == 32) {
        br_obj.br_index = 1;
    } else if (br == 64) {
        br_obj.br_index = 2;
    } else {
        log_error("opus_dec br set error! : %d\n", br);
    }
    decoder_ops_t *ops = get_opusdec_ops();
    ops->dec_confing(OPUS_CAL_BUF, SET_DEC_BR, &br_obj);

    set_opus_input_goon_cb(opus_goon_cb);
    return OPUS_DEC_INPUT_MAX_SIZE;
}
#endif
