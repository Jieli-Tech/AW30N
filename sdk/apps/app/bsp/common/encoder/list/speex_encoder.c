
#include "app_modules.h"

#if ENCODER_SPEEX_EN

#include "encoder_mge.h"
#include "cpu.h"
#include "config.h"
#include "typedef.h"
#include "hwi.h"
#include "speex_encode_api.h"

#include "circular_buf.h"


#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[speex_enc]"
#include "log.h"

const int SPEEXENC_SPEEDUP_EN = 6;	//0 ~ 6，配置speex编码加速，只对编码效果有影响，越高提速越多，效果变差
enc_obj enc_speex_hdl;
static cbuffer_t cbuf_enc_speex_o      AT(.enc_speex_data);
static u32 obuf_espeex_o[1024 / 4]	   AT(.enc_speex_data);
static u32 speex_encode_buff[8360 / 4] AT(.enc_speex_data);
static ENC_OPS spx_enc_ops             AT(.enc_speex_data);

static EN_FILE_IO speex_enc_io AT(.enc_speex_data);

/* quality 与 码率对应关系 */
/* sr = 8k；			|	sr = 16k; 		*/
/* 0	:	2kbps;		|	0	:	4kbps; 	*/
/* 1	:	4kbps;		|	1	:	6kbps; 	*/
/* 2	:	6kbps;		|	2	:	8kbps; 	*/
/* 3	:	8kbps;		|	3	:	10kbps; */
/* 4	:	8kbps;		|	4	:	12kbps; */
/* 5	:	11kbps;		|	5	:	16kbps; */
/* 6	:	11kbps;		|	6	:	20kbps; */
/* 7	:	15kbps;		|	7	:	24kbps; */
/* 8	:	15kbps;		|	8	:	28kbps; */
/* 9	:	18kbps;		|	9	:	34kbps; */

const u8 quality2br_8k_tab[10] = {
    2, 4, 6, 8, 8, 11, 11, 15, 15, 18
};
const u8 quality2br_16k_tab[10] = {
    4, 6, 8, 10, 12, 16, 20, 24, 28, 34
};

const u8 speex_sr_tab[] = {
    8000, 16000
};

u32 speex_encode_api(void *p_file, void *input_func, void *output_func)
{
    u32 buff_len;
    speex_enc_ops *ops;
    u32 ret;
    log_info("speex_encode_api\n");
    ops = get_speex_enc_obj();
    buff_len = ops->need_buf(16000);
    if (buff_len > sizeof(speex_encode_buff)) {
        log_info("speex need buf:%d %d\n", buff_len, sizeof(speex_encode_buff));
        return 0;
    }
    /******************************************/
    memset(&enc_speex_hdl, 0, sizeof(enc_obj));
    memset(&obuf_espeex_o[0], 0x00, sizeof(obuf_espeex_o));
    memset(&speex_encode_buff[0], 0x00, sizeof(speex_encode_buff));
    cbuf_init(&cbuf_enc_speex_o, &obuf_espeex_o[0], sizeof(obuf_espeex_o));

    enc_speex_hdl.p_file = p_file;
    enc_speex_hdl.p_ibuf = REC_ADC_CBUF; //adc_hdl.p_adc_cbuf;//&cbuf_ima_i;
    enc_speex_hdl.p_obuf = &cbuf_enc_speex_o;
    enc_speex_hdl.p_dbuf = &speex_encode_buff[0];

    memset(&spx_enc_ops, 0, sizeof(ENC_OPS));
    spx_enc_ops.run = ops->run;
    enc_speex_hdl.enc_ops = &spx_enc_ops;

    enc_speex_hdl.info.sr = read_audio_adc_sr();	//sr采样率:8k / 16k;
    for (u8 i = 0; i < sizeof(speex_sr_tab); i++) {
        if (enc_speex_hdl.info.sr == speex_sr_tab[i]) {//采样率匹配时退出
            break;
        }
        if (i == sizeof(speex_sr_tab)) {		//采样率都不匹配时return退出
            log_error("sbc sr not match %d\n", enc_speex_hdl.info.sr);
            return 0;
        }
    }
    enc_speex_hdl.info.nch = 1;
    u8 quality = 5;		//quality档位：0 ~ 9
    if (8000 == enc_speex_hdl.info.sr) {
        enc_speex_hdl.info.br = quality2br_8k_tab[quality];
    } else if (16000 == enc_speex_hdl.info.sr) {
        enc_speex_hdl.info.br = quality2br_16k_tab[quality];
    } else {
        log_error("speex_enc sr err!\n");
        return 0;
    }

    speex_enc_io.priv        = &enc_speex_hdl;
    speex_enc_io.input_data  = input_func;
    speex_enc_io.output_data = output_func;

    log_info("speex encode info:");
    log_info("sr : %d\n", enc_speex_hdl.info.sr);
    log_info("br : %d\n", enc_speex_hdl.info.br);

    /******************************************/
    if (ops->open((void *)&speex_encode_buff[0], &speex_enc_io, quality, enc_speex_hdl.info.sr)) {        //传入io接口，说明如下
        log_info("open fail\n");
        return 0;
    } else {
        log_info("speex open succ\n");
    }

    return (u32)&enc_speex_hdl;
}

#endif

