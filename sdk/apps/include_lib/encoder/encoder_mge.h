#ifndef __REC_API_H__
#define __REC_API_H__

#include "typedef.h"
#include "audio_enc_api.h"
#include "sound_mge.h"
#include "audio_adc.h"


#define B_ENC_ENABLE  BIT(0)
#define B_ENC_STOP    BIT(1)
#define B_ENC_FULL    BIT(2)
#define B_ENC_INIT    BIT(3)

typedef struct _enc_obj {
    void *p_file;
    void *p_ibuf;
    void *p_obuf;
    void *enc_ops;
    void *p_dbuf;
    ENC_DATA_INFO info;
    volatile u32 enable;
} enc_obj;

typedef enum  {//停止编码时，是否需要将ADC中剩余的样点消耗完
    ENC_NO_WAIT = 0,
    ENC_NEED_WAIT = 1,
} ENC_STOP_WAIT;

u16 enc_input(void *priv, s16 *buf, u8 channel, u16 len);
u32 enc_output(void *priv, u8 *data, u16 len);

void stop_encode(void *pfile, u32 dlen);
void stop_encode_phy(ENC_STOP_WAIT wait, u8 has_file_write);
void encoder_io(u32(*fun)(void *, void *, void *), void *pfile);
void wfil_soft2_isr_hook(enc_obj *hdl);

void kick_encode_api(void *obj);
void enc_phy_init(void);
void rec_cbuf_init(void *cbuf_t);


#define  kick_encode_isr()   bit_set_swi(1)
#define  kick_wfile_isr()    bit_set_swi(2)


// #if FPGA
#ifdef FPGA
typedef struct _adc_obj {
    void *p_adc_cbuf;
    volatile u32 enable;
} adc_obj;

#define B_DAC_ENABLE  BIT(0)
extern adc_obj adc_hdl;
#define REC_ADC_CBUF  adc_hdl.p_adc_cbuf

#else
extern sound_out_obj rec_sound;
#define REC_ADC_CBUF  rec_sound.p_obuf
#endif

#endif
