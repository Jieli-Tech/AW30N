#include "app_modules.h"

#if ENCODER_OPUS_EN

#include "opus_encoder.h"
#include "encoder_mge.h"
#include "cpu.h"
#include "config.h"
#include "typedef.h"
#include "hwi.h"

#include "circular_buf.h"
#include "opus_enc_api.h"

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[opus_enc]"
#include "log.h"

static enc_obj enc_opus_hdl;
static cbuffer_t cbuf_enc_opus_o        AT(.enc_opus_data);
static u8 obuf_enc_opus_o[1024]         AT(.enc_opus_data);
static u32 opus_encode_buff[20296 / 4]  AT(.enc_opus_data);
static ENC_OPS opus_enc_ops             AT(.enc_opus_data);

static u8 opus_quality_sel(u8 br, u8 complexity, u8 format_mode)
{
    u8 br_sel = 0;

    if (br == 16) {
        br_sel = QY_BR_CONFIG_16KBPS;
    } else if (br == 32) {
        br_sel = QY_BR_CONFIG_32KBPS;
    } else if (br == 64) {
        br_sel = QY_BR_CONFIG_64KBPS;
    } else {
        log_error("OPUS encode bitrate error!!!\n");
        return 0;
    }

    u8 quality = (br_sel << QY_BR_CONFIG_BIT_OFFSET) | \
                 (complexity << QY_COMPLEXITY_BIT_OFFSET) | \
                 (format_mode << QY_FORMAT_MODE_BIT_OFFSET);

    return quality;
}
/* static u16 opus_enc_input(void *priv, s16 *buf, u16 len) */
/* { */
/*     u16 res = enc_input(priv, buf, 1, len); */
/*     #<{(| static u32 cnt = 0; |)}># */
/*     #<{(| log_info("%d in:%d %d", cnt++, res, len); |)}># */
/*     return res; */
/* } */
/* static u32(*opus_enc_output_cb)(void *, u8 *, u16) = NULL; */
/* static u32 opus_enc_output(void *priv, u8 *data, u16 len) */
/* { */
/*     u32 res = 0; */
/*     if (opus_enc_output_cb) { */
/*         res = opus_enc_output_cb(priv, data, len); */
/*     } else { */
/*         res = enc_output(priv, data, len); */
/*     } */
/*     #<{(| static u32 cnt = 0; |)}># */
/*     #<{(| log_info("%d out:%d %d", cnt++, res, len); |)}># */
/*     return res; */
/* } */
/* void opus_enc_output_cb_sel(u32 opus_enc_output_func(void *, u8 *, u16)) */
/* { */
/*     local_irq_disable(); */
/*     opus_enc_output_cb = opus_enc_output_func; */
/*     local_irq_enable(); */
/* } */
static EN_FILE_IO opus_enc_io AT(.enc_opus_data);
/* static const OPUS_EN_FILE_IO opus_enc_io = { */
/*     &enc_opus_hdl,      //input跟output函数的第一个参数，解码器不做处理，直接回传，可以为NULL */
/*     opus_enc_input,     //opus input与通用编码接口有差异，需封装一层 */
/*     opus_enc_output, */
/* }; */

u32 opus_encode_api(void *p_file, void *input_func, void *output_func)
{
    u32 buff_len, i;
    OPUS_ENC_OPS *ops;
    log_info("opus_encode_api\n");
    ops = get_opus_enc_ops();
    buff_len = ops->need_buf(16000);//固定16k采样率
    if (buff_len > sizeof(opus_encode_buff)) {
        log_info("opus need buf:%d %d\n", buff_len, sizeof(opus_encode_buff));
        return 0;
    }
    /******************************************/
    memset(&enc_opus_hdl, 0, sizeof(enc_obj));
    memset(&obuf_enc_opus_o[0], 0x00, sizeof(obuf_enc_opus_o));
    memset(&opus_encode_buff[0], 0x00, sizeof(opus_encode_buff));
    cbuf_init(&cbuf_enc_opus_o, &obuf_enc_opus_o[0], sizeof(obuf_enc_opus_o));
    log_info("A\n");
    // log_info("B\n");
    opus_enc_io.priv        = &enc_opus_hdl;
    opus_enc_io.input_data  = input_func;
    opus_enc_io.output_data = output_func;
    enc_opus_hdl.p_file = p_file;
    enc_opus_hdl.p_ibuf = REC_ADC_CBUF;//adc_hdl.p_adc_cbuf;//&cbuf_emp3_i;
    enc_opus_hdl.p_obuf = &cbuf_enc_opus_o;
    enc_opus_hdl.p_dbuf = &opus_encode_buff[0];

    /* 统一opus run与标准编码的ops run接口 */
    memset(&opus_enc_ops, 0, sizeof(ENC_OPS));
    opus_enc_ops.run = ops->run;
    enc_opus_hdl.enc_ops = &opus_enc_ops;

    enc_opus_hdl.info.sr = 16000;//固定16k采样率
    enc_opus_hdl.info.br = 16;//16kbps、32kbps、64kbps
    enc_opus_hdl.info.nch = 1;
    if (enc_opus_hdl.info.sr != read_audio_adc_sr()) {
        log_error("opus sr not match %d %d \n", enc_opus_hdl.info.sr, read_audio_adc_sr());
        return 0;
    }

    u8 quality_complexity = QY_COMPLEXITY_LOW;
    u8 quality_format_mode = QY_FORMAT_MODE_BAIDU;
    u8 quality = opus_quality_sel(enc_opus_hdl.info.br, quality_complexity, quality_format_mode);
    log_info("opus quality:%d = br:%d + complex:%d + format:%d\n", \
             quality, \
             enc_opus_hdl.info.br, \
             quality_complexity, \
             quality_format_mode);

    log_info("D\n");
    /******************************************/
    enc_opus_hdl.enable |= B_ENC_INIT;
    ops->open((void *)&opus_encode_buff[0], (void *)&opus_enc_io, quality, enc_opus_hdl.info.sr);
    enc_opus_hdl.enable &= ~B_ENC_INIT;
    return (u32)&enc_opus_hdl;
}

#if 0   //性能评估
const u8 opus_parm_tab[][3] = {
    /* 16kbps */
    {QY_BR_CONFIG_16KBPS, QY_FORMAT_MODE_BAIDU, QY_COMPLEXITY_HIGH},
    {QY_BR_CONFIG_16KBPS, QY_FORMAT_MODE_BAIDU, QY_COMPLEXITY_LOW},
    {QY_BR_CONFIG_16KBPS, QY_FORMAT_MODE_KUGOU, QY_COMPLEXITY_HIGH},
    {QY_BR_CONFIG_16KBPS, QY_FORMAT_MODE_KUGOU, QY_COMPLEXITY_LOW},
    {QY_BR_CONFIG_16KBPS, QY_FORMAT_MODE_OGG,   QY_COMPLEXITY_HIGH},
    {QY_BR_CONFIG_16KBPS, QY_FORMAT_MODE_OGG,   QY_COMPLEXITY_LOW},

    /* 32kbps */
    {QY_BR_CONFIG_32KBPS, QY_FORMAT_MODE_BAIDU, QY_COMPLEXITY_HIGH},
    {QY_BR_CONFIG_32KBPS, QY_FORMAT_MODE_BAIDU, QY_COMPLEXITY_LOW},
    {QY_BR_CONFIG_32KBPS, QY_FORMAT_MODE_KUGOU, QY_COMPLEXITY_HIGH},
    {QY_BR_CONFIG_32KBPS, QY_FORMAT_MODE_KUGOU, QY_COMPLEXITY_LOW},
    {QY_BR_CONFIG_32KBPS, QY_FORMAT_MODE_OGG,   QY_COMPLEXITY_HIGH},
    {QY_BR_CONFIG_32KBPS, QY_FORMAT_MODE_OGG,   QY_COMPLEXITY_LOW},

    /* 64kbps */
    {QY_BR_CONFIG_64KBPS, QY_FORMAT_MODE_BAIDU, QY_COMPLEXITY_HIGH},
    {QY_BR_CONFIG_64KBPS, QY_FORMAT_MODE_BAIDU, QY_COMPLEXITY_LOW},
    {QY_BR_CONFIG_64KBPS, QY_FORMAT_MODE_KUGOU, QY_COMPLEXITY_HIGH},
    {QY_BR_CONFIG_64KBPS, QY_FORMAT_MODE_KUGOU, QY_COMPLEXITY_LOW},
    {QY_BR_CONFIG_64KBPS, QY_FORMAT_MODE_OGG,   QY_COMPLEXITY_HIGH},
    {QY_BR_CONFIG_64KBPS, QY_FORMAT_MODE_OGG,   QY_COMPLEXITY_LOW},
};
u8 opus_quality = 0;
u8 quality_cnt = 0;
void opus_parm_sel(char c)
{
    if (c == '+') {
        quality_cnt++;
        if (quality_cnt >= 18) {
            quality_cnt = 0;
        }
    } else if (c == '-') {
        quality_cnt--;
        if (quality_cnt >= 18) {
            quality_cnt = 17;
        }
    }

    u8 qy_br = opus_parm_tab[quality_cnt][0];
    u8 qy_format_mode = opus_parm_tab[quality_cnt][1];
    u8 qy_complexity = opus_parm_tab[quality_cnt][2];
    opus_quality = 0;
    opus_quality = qy_br | qy_format_mode | qy_complexity;
    log_info("num:%d br:%d mode:%d cmlx:%d quality:0x%x\n", quality_cnt, qy_br, qy_format_mode >> 6, qy_complexity >> 4, opus_quality);
}
#endif
#endif
