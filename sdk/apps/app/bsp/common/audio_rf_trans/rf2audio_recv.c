#pragma bss_seg(".ar_recv.data.bss")
#pragma data_seg(".ar_recv.data")
#pragma const_seg(".ar_recv.text.const")
#pragma code_seg(".ar_recv.text")
#pragma str_literal_override(".ar_recv.text.const")

#include "rf2audio_recv.h"
#include "audio_rf_mge.h"
#include "app_modules.h"
#include "circular_buf.h"
#include "cpu.h"
#include "config.h"
#include "typedef.h"
#include "vfs.h"
#include "msg.h"
#include "crc16.h"
#include "usb_mic_interface.h"

#include "decoder_api.h"
#include "decoder_mge.h"
#include "sound_mge.h"
#include "app.h"
#include "audio.h"
#include "audio_adc_api.h"
#include "audio_adc.h"
#include "audio_dac_api.h"
#include "audio_dac.h"
#include "src_api.h"
#include "hwi.h"

#if DECODER_UMP3_EN
#include "ump3_api.h"
#endif
#if DECODER_OPUS_EN
#include "opus_api.h"
#endif

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[rf_recv]"
#include "log.h"

u32 rf2audio_dec_null_func(void *priv)
{
    return -1;
}

static int rf2audio_receiver_mp_input(void *priv, u32 addr, void *buf, int len, u8 type);
const struct if_decoder_io dec_io_for_rf = {
    0,
    rf2audio_receiver_mp_input,
    0,
    mp_output,
    rf2audio_dec_null_func,
    0
};


/* 内部变量定义 */
sound_stream_obj rf_stream AT(.ar_trans_data);
cbuffer_t cbuf_rf_data AT(.ar_trans_data);
volatile dec_obj *g_recv_dec_obj AT(.ar_trans_data);
static u32 rf_data_buff[1024 * 2 / 4] AT(.ar_trans_data);
volatile u16 rf_dec_kick_size AT(.ar_trans_data);

/*----------------------------------------------------------------------------*/
/**@brief   启动解码器解码数据
   @param   p_stream_in :数据流管理句柄，该参数为空强制启动一次解码
            psound      :音频数据句柄
   @return
   @author
   @note    接收数据并写入缓存后，启动解码器去缓存中读取数据
            启动前会判断缓存数据是否满足一帧，满足则启动解码，不满足则退出
*/
/*----------------------------------------------------------------------------*/
void rf2audio_receiver_kick_decoder(void *p_stream_in, void *psound)
{
    sound_stream_obj *pstream = (sound_stream_obj *)p_stream_in;
    if (NULL != pstream) {
        u32 len = cbuf_get_data_size((cbuffer_t *)pstream->p_ibuf);
        if (len < pstream->kick_thr) {
            /* 未达到数据源kick门槛值 */
            return;
        }
    }
    if (NULL != psound) {
        sound_out_obj *p_sound = psound;
        p_sound->enable |= B_DEC_KICK;
        kick_decoder();
    }
}

/*----------------------------------------------------------------------------*/
/**@brief   写入音频接收缓存数据
   @param   data    :写入的数据buff
            data_len:写入的数据长度
   @return  本次已成功写入的数据长度
   @author
   @note    音频接收缓存cbuf为rf_data_buff，由cbuf_rf_data管理，收到无线传输数据后写入
*/
/*----------------------------------------------------------------------------*/
u32 rf2audio_receiver_write_data(u8 *data, u32 data_len)
{
    if (0 == if_decoder_is_run(g_recv_dec_obj)) {
        return 0;
    }
    if (B_DEC_ERR == decoder_status(g_recv_dec_obj)) {
        return 0;
    }
    if (cbuf_get_available_space(&cbuf_rf_data) < data_len) {
        rf2audio_receiver_kick_decoder(g_recv_dec_obj->p_file, &g_recv_dec_obj->sound);
        return 0;
    }
    u32 wlen = 0;
    wlen = cbuf_write(&cbuf_rf_data, data, data_len);
    if (wlen != data_len) {
        log_info("RF_rxbuf Full!!!\n");
        /* while(1); */
    }
    rf2audio_receiver_kick_decoder(g_recv_dec_obj->p_file, &g_recv_dec_obj->sound);
    return wlen;
}
/*----------------------------------------------------------------------------*/
/**@brief   读取音频接收缓存数据
   @param   data    :读取的数据buff
            data_len:读取的数据长度
   @return  本次已成功读取的数据长度
   @author
   @note    音频接收缓存cbuf为rf_data_buff，由cbuf_rf_data管理，解码器启动后读取
*/
/*----------------------------------------------------------------------------*/
u32 rf2audio_receiver_read_data(u8 *data, u32 data_len)
{
    u32 remain_len = cbuf_get_data_size(&cbuf_rf_data);
    u32 rlen = cbuf_read(&cbuf_rf_data, data, (data_len > remain_len ? remain_len : data_len));
    return rlen;
}

/*----------------------------------------------------------------------------*/
/**@brief   解码器数据预读回调
   @param   priv    :私有结构体，此处为解码句柄
   @return  告知解码器可提供的数据长度
   @author
   @note    返回长度小于解码需要的最小长度，会跳过本次解码
*/
/*----------------------------------------------------------------------------*/
static int rf2audio_receiver_read_check(void *priv)
{
    u32 len = cbuf_get_data_size(&cbuf_rf_data);
    /* log_info("check: %d", len); */
    return len;
}
/*----------------------------------------------------------------------------*/
/**@brief   解码器数据输入回调
   @param   priv    :私有结构体，此处为解码句柄
            addr    :解码读取的文件位置，无线传输中忽略该参数
            buf     :解码读取数据buff
            len     :解码读取数据长度
            type    :保留
   @return  返回本次提供给解码器的数据长度
   @author
   @note
*/
/*----------------------------------------------------------------------------*/
static int rf2audio_receiver_mp_input(void *priv, u32 addr, void *buf, int len, u8 type)
{
    int rlen = rf2audio_receiver_read_data(buf, len);
    /* static u32 cnt = 0; */
    /* log_info("in %d %d %d", cnt++, rlen, len); */
    return rlen;
}
/*----------------------------------------------------------------------------*/
/**@brief   音频传输接收开始，启动解码器
   @param   p_enc_head  :编码头信息，依据该信息初始化解码器
            channel     :解码后的pcm数据输出端口，可选择输出到DAC或USB MIC
            output_sr   :解码后的pcm数据输出采样率
   @return
   @author
   @note    音频传输接收到编码头信息后，启动相应解码器开始解码
*/
/*----------------------------------------------------------------------------*/
bool rf2audio_decoder_start(RF_RADIO_ENC_HEAD *p_enc_head, STREAM_CHANNEL channel, u32 output_sr)
{
    set_radio_status(RADIO_RECEIVING);
    cbuf_init(&cbuf_rf_data, &rf_data_buff[0], sizeof(rf_data_buff));

    log_info("/***********************/");
    log_info("RF_Recv_Head Succ!");
    log_info("Encode_Type : %d", p_enc_head->enc_type);
    log_info("Sample_Rate : %d", p_enc_head->sr);
    log_info("Bit_Rate    : %d", p_enc_head->br);
    log_info("Reserved    : %d", p_enc_head->reserved);
    log_info("/***********************/\n");

    memset(&rf_stream, 0, sizeof(sound_stream_obj));
    rf_stream.p_ibuf    = &cbuf_rf_data;

    dec_data_stream t_strm = {0};
    t_strm.strm_source = &rf_stream;
    t_strm.io   = (struct if_decoder_io *)&dec_io_for_rf;
    t_strm.goon_callback = rf2audio_receiver_read_check;
    t_strm.sr   = p_enc_head->sr;
    t_strm.br   = p_enc_head->br;
    t_strm.strm_ctl = B_DEC_NO_CHECK | B_DEC_IS_STRM;

    g_recv_dec_obj = decoder_list(&t_strm, BIT(p_enc_head->enc_type), 0, 0, output_sr, &rf_dec_kick_size);
    if (NULL == g_recv_dec_obj) {
        log_info("rf_recv init fail \n");
        return false;
    }

    g_recv_dec_obj->p_kick = rf2audio_receiver_kick_decoder;
    if (channel == STREAM_TO_USB) {
        set_usb_mic_info(&g_recv_dec_obj->sound, g_recv_dec_obj->p_kick, g_recv_dec_obj->src_effect);
    } else {
        regist_dac_channel(&g_recv_dec_obj->sound, NULL);
    }
    g_recv_dec_obj->sound.enable |= B_DEC_ENABLE | B_DEC_FIRST;

    log_info("rf_recv init succ \n");
    return true;
}
/*----------------------------------------------------------------------------*/
/**@brief   音频传输接收停止，关闭解码器
   @param
   @return
   @author
   @note
*/
/*----------------------------------------------------------------------------*/
void rf2audio_decoder_stop(STREAM_CHANNEL channel)
{
    if (channel == STREAM_TO_USB) {
        decoder_stop_phy(g_recv_dec_obj, NO_WAIT, 0, 1, clr_usb_mic_info);
    } else {
        decoder_stop_phy(g_recv_dec_obj, NO_WAIT, 0, 1, unregist_dac_channel);
    }
    local_irq_disable();
    g_recv_dec_obj = NULL;
    local_irq_enable();
}

sound_out_obj *usb_mic_get_stream_info(u32 sr, u32 frame_len, u32 ch, void **ppsrc, void **pkick)
{
    if (NULL == g_recv_dec_obj) {
        return NULL;
    }
    *ppsrc = g_recv_dec_obj->src_effect;
    *pkick = g_recv_dec_obj->p_kick;
    return &g_recv_dec_obj->sound;
}

void usb_mic_stream_clear_stream_info(void **ppsrc)
{

}


#if 0	//RF_RECV TEST

#include "hwi.h"
#include "irq.h"

const u8 ump3_data_test[] = {
    0x0F, 0x15, 0x00, 0xA3, 0x7F, 0x5F, 0xC0, 0x1D, 0x9F, 0x20, 0xB1, 0x39, 0xC7, 0x17, 0x16, 0x04,
    0x44, 0x6E, 0xF0, 0x75, 0xE4, 0xBA, 0x24, 0xDF, 0x3F, 0xFB, 0x3F, 0x16, 0xE0, 0xF1, 0x0F, 0x37,
    0x68, 0xD3, 0xB3, 0x2E, 0x86, 0xAB, 0xC6, 0xFC, 0x01, 0xD9, 0x9E, 0x62, 0xE3, 0x10, 0x36, 0x78,
    0xE8, 0xF7, 0xF3, 0x05, 0x73, 0xC5, 0x09, 0xBD, 0x3D, 0x2C, 0x02, 0xDF, 0xFC, 0x7E, 0x3F, 0x1F,
    0x50, 0x97, 0x54, 0xC0, 0x74, 0x3D, 0xDF, 0x9F, 0xDF, 0x0F, 0xE8, 0x33, 0xEA, 0xE4, 0x9A, 0x85,
    0x0F, 0xE7, 0xFB, 0xF2, 0xFB, 0xFD, 0x82, 0xBF, 0x62, 0xEF, 0xBE, 0x24, 0x06, 0x83, 0x67, 0x7E,
    0xBF, 0xDF, 0x0F, 0x67, 0x3C, 0x40, 0xEF, 0x3B, 0xE3, 0xEF, 0xC4, 0xEF, 0xF7, 0xF3, 0xF5, 0x70,
    0x2D, 0xC3, 0x0A, 0xD7, 0xF3, 0x01, 0x00, 0x00
};

#define TIMER_SFR(ch) JL_TIMER##ch
#define _timer_init(ch,ms)  \
    request_irq(IRQ_TIME##ch##_IDX, 7, timer##ch##_isr, 0); \
	SFR(TIMER_SFR(ch)->CON, 10, 4, 5);                      \
    SFR(TIMER_SFR(ch)->CON, 4, 4, 0b1000);                       \
	TIMER_SFR(ch)->CNT = 0;                                     \
	TIMER_SFR(ch)->PRD = 12000000 / 1000 / (1 * 256) * ms;	       \
	TIMER_SFR(ch)->CON |= BIT(14);                              \
	TIMER_SFR(ch)->CON |= BIT(0);


static u8 timer_flag = 0;
SET(interrupt(""))
AT_RAM
static void timer0_isr(void)
{
    IO_DEBUG_TOGGLE(B, 4);
    TIMER_SFR(0)->CON |= BIT(14);
    /* putchar('1'); */
    if (timer_flag == 0) {
        rf_receiver_start(32000, 80);
        timer_flag = 1;
        return;
    }
    rf_receiver_write_data(&ump3_data_test[0], sizeof(ump3_data_test));
    rf_receiver_kick_decoder();
}

void timer_init(u8 timer_ch, u32 ms)
{
    switch (timer_ch) {
    case 0:
        _timer_init(0, ms);
        break;
    case 1:
        /* _timer_init(1, ms); */
        break;
    case 2:
        /* _timer_init(2, ms); */
        break;
    default:
        break;
    }
}
#endif
