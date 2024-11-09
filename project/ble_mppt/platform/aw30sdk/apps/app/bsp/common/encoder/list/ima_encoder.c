#include "app_modules.h"

#if ENCODER_IMA_EN

#include "encoder_mge.h"
#include "cpu.h"
#include "config.h"
#include "typedef.h"
#include "hwi.h"
#include "circular_buf.h"

#include "ima_stream_enc_api.h"

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[ima_enc]"
#include "log.h"

enc_obj enc_ima_hdl;
static cbuffer_t cbuf_enc_ima_o      AT(.enc_ima_data);
static u32 obuf_eima_o[1024 / 4]	 AT(.enc_ima_data);
static u32 ima_encode_buff[820 / 4]	 AT(.enc_ima_data);

static EN_FILE_IO ima_enc_io 		 AT(.enc_ima_data);
const int ima_enc_stream_header = 0;        //编码输出header 0:不输出; 1:输出
const int ima_enc_stream_max_input = 160;	//样点数 320 byte
const int ima_enc_stream_max_output = 40 * 2; //byte

u32 ima_encode_api(void *p_file, void *input_func, void *output_func)
{
    log_info("ima_encode_api\n");
    u32 buff_len, i;
    ENC_OPS *ops;
    ops = get_ima_adpcm_stream_code_ops();

    u32 ret;
    buff_len = ops->need_buf();
    if (buff_len > sizeof(ima_encode_buff)) {
        log_info("ima need buf:%d %d\n", buff_len, sizeof(ima_encode_buff));
        return 0;
    }
    /******************************************/
    memset(&enc_ima_hdl, 0, sizeof(enc_obj));
    memset(&obuf_eima_o[0], 0x00, sizeof(obuf_eima_o));
    memset(&ima_encode_buff[0], 0x00, sizeof(ima_encode_buff));
    cbuf_init(&cbuf_enc_ima_o, &obuf_eima_o[0], 1024);
    /* debug_puts("A\n"); */

    enc_ima_hdl.p_file = p_file;
    enc_ima_hdl.p_ibuf = REC_ADC_CBUF; //adc_hdl.p_adc_cbuf;//&cbuf_ima_i;
    enc_ima_hdl.p_obuf = &cbuf_enc_ima_o;
    enc_ima_hdl.p_dbuf = &ima_encode_buff[0];
    enc_ima_hdl.enc_ops = ops;
    enc_ima_hdl.info.sr = read_audio_adc_sr();
    enc_ima_hdl.info.br = enc_ima_hdl.info.sr * 16 / 4 / 1000;
    log_info("ima encode info:");
    log_info("ima sr:%d br:%d\n", enc_ima_hdl.info.sr, enc_ima_hdl.info.br);


    ima_enc_io.priv        = &enc_ima_hdl;
    ima_enc_io.input_data  = input_func;
    ima_enc_io.output_data = output_func;

    /******************************************/
    ops->open((void *)&ima_encode_buff[0], (void *)&ima_enc_io);          //传入io接口，说明如下
    return (u32)&enc_ima_hdl;
}

#endif
