
#include "app_modules.h"

#if ENCODER_SBC_EN

#include "sbc_encoder.h"
#include "encoder_mge.h"
#include "cpu.h"
#include "config.h"
#include "typedef.h"
#include "hwi.h"
#include "circular_buf.h"

#include "sbc_stdenc_api.h"

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[sbc_enc]"
#include "log.h"

#define ENC_SBC			0
#define ENC_MSBC		1
#define ENC_USBC		2
#define ENC_SBC_TYPE	ENC_SBC

enc_obj enc_sbc_hdl;
static cbuffer_t cbuf_enc_sbc_o      	AT(.enc_sbc_data);
static u32 obuf_esbc_o[1024 / 4]	   	AT(.enc_sbc_data);
static u32 sbc_encode_buff[4088 / 4] 	AT(.enc_sbc_data);
static ENC_OPS sbc_enc_ops             	AT(.enc_sbc_data);

static EN_FILE_IO sbc_enc_io AT(.enc_sbc_data);

const u8 sbc_sr_tab[] = {
    16000, 32000, 44100, 48000
};


u32 sbc_encode_api(void *p_file, void *input_func, void *output_func)
{
    log_info("sbc_encode_api\n");
    u32 buff_len;
    u32 ret;
    SBC_STENC_OPS *ops = get_sbc_stenc_ops();
    cbuf_init(&cbuf_enc_sbc_o, &obuf_esbc_o[0], sizeof(obuf_esbc_o));

    /* 具体参数信息在头文件sbc_stdenc_api.h处 */
    SBC_ENC_PARA sbc_para_set;
    sbc_para_set.msbc = ENC_SBC_TYPE;		//0：编码sbc; 1：编码msbc; 2：编码usbc
    if ((sbc_para_set.msbc == ENC_SBC) || (sbc_para_set.msbc == ENC_USBC)) {		//选择sbc 或 usbc时
        sbc_para_set.bitpool = 31;
        sbc_para_set.subbands = 8;
        sbc_para_set.blocks = 16;
        sbc_para_set.joint = 0;
        sbc_para_set.nch = 1;
        sbc_para_set.dual = 1;
        sbc_para_set.snr = 1;
        sbc_para_set.sr = read_audio_adc_sr();
        for (u8 i = 0; i < sizeof(sbc_sr_tab); i++) {
            if (sbc_para_set.sr == sbc_sr_tab[i]) {//采样率匹配时退出
                break;
            }
            if (i == sizeof(sbc_sr_tab)) {		//sbc 或 usbc 时，采样率都不匹配时return退出
                log_error("sbc sr not match %d\n", sbc_para_set.sr);
                return 0;
            }
        }
    } else if (sbc_para_set.msbc == ENC_MSBC) {
        /* 选择msbc时，参数库内固定 */
        /* sr=16000,nch=1,bitpool=26,subbands=8,blocks=15; */
        sbc_para_set.sr = read_audio_adc_sr();
        if (sbc_para_set.sr != 16000) {			//msbc时，采样率不匹配时return退出
            log_error("msbc sr not match \n", sbc_para_set.sr);
            return 0;
        }
    }

    buff_len = ops->need_buf(&sbc_para_set);
    if (buff_len > sizeof(sbc_encode_buff)) {
        log_error("sbc need buf:%d %d\n", buff_len, sizeof(sbc_encode_buff));
        return 0;
    }

    memset(&enc_sbc_hdl, 0, sizeof(enc_obj));
    sbc_enc_io.priv        = &enc_sbc_hdl;
    sbc_enc_io.input_data  = input_func;
    sbc_enc_io.output_data = output_func;

    enc_sbc_hdl.p_file = p_file;
    enc_sbc_hdl.p_ibuf = REC_ADC_CBUF;
    enc_sbc_hdl.p_obuf = &cbuf_enc_sbc_o;
    enc_sbc_hdl.p_dbuf = &sbc_encode_buff[0];

    if ((sbc_para_set.msbc == ENC_SBC) || (sbc_para_set.msbc == ENC_USBC)) {
        enc_sbc_hdl.info.sr = read_audio_adc_sr();
    } else if (sbc_para_set.msbc == ENC_MSBC) {
        enc_sbc_hdl.info.sr = 16000;	//msbc固定采样率为16000
    }

    memset(&sbc_enc_ops, 0, sizeof(ENC_OPS));
    sbc_enc_ops.run = ops->run;
    enc_sbc_hdl.enc_ops = &sbc_enc_ops;

    if (ops->open((void *)&sbc_encode_buff[0], &sbc_enc_io, &sbc_para_set)) { //传入io接口
        log_error("open fail\n");
        return 0;
    } else {
        log_info("sbc open succ\n");
    }

    /* 获取编码输入pcm点数和压缩后的输出帧长需要在open后获取 */
    enc_sbc_hdl.indata_kick_size = ops->config((void *)&sbc_encode_buff[0], GET_ENC_PCMPOINT_PERCH, 0);	//获取算法输入pcm点数
    u32 outdata_size = ops->config((void *)&sbc_encode_buff[0], GET_ENCODE_FRLEN, 0);	//获取算法输出帧长
    log_info("indata_size %d, outdata_size %d\n", enc_sbc_hdl.indata_kick_size, outdata_size);

    if ((sbc_para_set.msbc == ENC_SBC) || (sbc_para_set.msbc == ENC_USBC)) {
        /* sbc 或 usbc 时计算码率br */
        enc_sbc_hdl.info.br = (8 * outdata_size * enc_sbc_hdl.info.sr) / (sbc_para_set.subbands * sbc_para_set.blocks * 1000);
    } else if (sbc_para_set.msbc == ENC_MSBC) {
        /* msbc时计算码率br */
        enc_sbc_hdl.info.br = (8 * outdata_size * enc_sbc_hdl.info.sr) / (8 * 15 * 1000);
    }

    log_info("sbc encode info:");
    log_info("sr : %d\n", enc_sbc_hdl.info.sr);
    log_info("br : %d\n", enc_sbc_hdl.info.br);

    return (u32)&enc_sbc_hdl;
}

#endif

