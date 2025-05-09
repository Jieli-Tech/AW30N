
#include "app_modules.h"

#if ENCODER_A_EN

#include "encoder_mge.h"
#include "cpu.h"
#include "config.h"
#include "typedef.h"
#include "hwi.h"
#include "a_encode_lib.h"
#include "lib_enc_a.h"

#include "circular_buf.h"


#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[a_enc]"
#include "log.h"

/* ------------------------------------------------------------- */
/* |A编码 码率（BR）/采样率（SR）对应关系                      | */
/* ------------------------------------------------------------- */
/* |采样率(SR)  |   8k      12k     16k     24k     32k        | */
/* |码率(BR)    |   32kbps  48kbps  64kbps  96kbps  128kbps    | */
/* ------------------------------------------------------------- */
/* |注：A格式编码码率与采样率保持4：1的关系                    | */
/* ------------------------------------------------------------- */

const u16 a_enc_sr_tab[] = {
    8000,
    12000,
    16000,
    24000,
    32000
};


cbuffer_t cbuf_ima_o    AT(.enc_a_data);
u8 obuf_ima_o[1024]     AT(.enc_a_data);
u32 a_encode_buff[A_EBUF_SIZE / 4] AT(.enc_a_data) ;

enc_obj enc_a_hdl;

static EN_FILE_IO a_enc_io AT(.enc_a_data);
/* const EN_FILE_IO a_enc_io = { */
/*     &enc_a_hdl,      //input跟output函数的第一个参数，解码器不做处理，直接回传，可以为NULL */
/*     enc_input, */
/*     enc_output, */
/* }; */

u32 a_encode_api(void *p_file, void *input_func, void *output_func)
{
    u32 buff_len, i;
    ENC_OPS *ops;
    log_info("a_encode_api\n");
    ops = get_ima_code_ops();
    buff_len = ops->need_buf();
    if (buff_len > sizeof(a_encode_buff)) {
        log_info("a buff_len : %d", buff_len);
        log_info("a a_encode_buff len : %d", sizeof(a_encode_buff));
        return 0;
    }
    /******************************************/

    memset(&enc_a_hdl, 0, sizeof(enc_obj));
    memset(&obuf_ima_o[0], 0x00, sizeof(obuf_ima_o));
    memset(&a_encode_buff[0], 0x00, sizeof(a_encode_buff));
    cbuf_init(&cbuf_ima_o, &obuf_ima_o[0], sizeof(obuf_ima_o));
    /* debug_puts("A\n"); */
    // debug_puts("B\n");
    a_enc_io.priv        = &enc_a_hdl;
    a_enc_io.input_data  = input_func;
    a_enc_io.output_data = output_func;
    enc_a_hdl.p_file = p_file;
    /* debug_u32hex((u32)p_file); */
    enc_a_hdl.p_ibuf = REC_ADC_CBUF; //adc_hdl.p_adc_cbuf;//&cbuf_ima_i;
    enc_a_hdl.p_obuf = &cbuf_ima_o;
    enc_a_hdl.p_dbuf = &a_encode_buff[0];
    enc_a_hdl.enc_ops = ops;
    enc_a_hdl.info.sr = read_audio_adc_sr();
    for (i = 0; i < ARRAY_SIZE(a_enc_sr_tab); i++) {
        if (enc_a_hdl.info.sr == a_enc_sr_tab[i]) {
            break;
        }
    }
    if (i == ARRAY_SIZE(a_enc_sr_tab)) {
        log_error("a encode sample rate %d is not matched!\n", enc_a_hdl.info.sr);
        return 0;
    }
    enc_a_hdl.info.br = enc_a_hdl.info.sr / 1000 * 4;
    enc_a_hdl.info.nch = 1;

    log_info("a encode info:");
    log_info("sr : %d", enc_a_hdl.info.sr);
    log_info("br : %dk\n", enc_a_hdl.info.br);

    /* debug_puts("D\n"); */
    /******************************************/
    ops->open((void *)&a_encode_buff[0], (void *)&a_enc_io);          //传入io接口，说明如下
    ops->set_info((void *)&a_encode_buff[0], &enc_a_hdl.info);
    ops->init((void *)&a_encode_buff[0]);
    return (u32)&enc_a_hdl;
}

#endif
