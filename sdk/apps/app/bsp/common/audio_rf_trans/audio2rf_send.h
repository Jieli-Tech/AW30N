#ifndef __RF_SENDER_H__
#define __RF_SENDER_H__
#include "typedef.h"
#include "audio_rf_mge.h"

#define RF_SENDER_USE_SOFT_ISR   0

extern void enc_soft1_isr(void);
extern void rec_phy_init(void);
extern void start_encode(void);

// void rf_sender_soft2_isr(void);
// void rf_sender_stop(void);
// void rf_sender_start(void);
// void rf_encoder_stop(void)
// u32 rf_enc_head_output(void *enc_handle, u8 enc_type);
u32 rf_enc_head_output(u32 sr, u32 br, u8 enc_type);
u32 rf_enc_output(void *priv, u8 *data, u16 len);
void audio2rf_encoder_stop(void);
bool audio2rf_encoder_io(u32(*enc_fun)(void *, void *, void *), void *input_func, void *output_func, u8 enc_type);
u32 audio2rf_send_data(u8 *data, u32 len);
u32 audio2rf_send_stop_packet(void);
u32 rf_send_data_api(u8 *data, u16 len, u8 data_type);
void audio2rf_send_register(int (*send_func)(u8 *, u16));
#endif
