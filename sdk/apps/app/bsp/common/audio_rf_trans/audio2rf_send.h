#ifndef __RF_SENDER_H__
#define __RF_SENDER_H__
#include "typedef.h"
#include "audio_rf_mge.h"
#include "audio_codec_format.h"
#include "encoder_mge.h"
#include "rf_send_queue.h"

extern void rec_phy_init(void);

// void rf_sender_soft2_isr(void);
// void rf_sender_stop(void);
// void rf_sender_start(void);
// void rf_encoder_stop(void)
// u32 rf_enc_head_output(void *enc_handle, u8 enc_type);
// u32 rf_enc_head_output(u32 sr, u32 br, AUDIO_FORMAT enc_type);
// u32 audio2rf_start_cmd(u32 sr, u32 br, AUDIO_FORMAT enc_type);
// u32 audio2rf_stop_cmd(void);
// u32 audio2rf_ack_cmd(u8 ack_cmd, u8 *data, u16 len);
u32 audio2rf_start_cmd(queue_obj *p_queue, u32 sr, u32 br, AUDIO_FORMAT enc_type);
u32 audio2rf_stop_cmd(queue_obj *p_queue);
u32 audio2rf_ack_cmd(queue_obj *p_queue, u8 ack_cmd, u8 *data, u16 len);

u32 rf_enc_output(void *priv, u8 *data, u16 len);
enc_obj *audio2rf_encoder_stop(enc_obj *p_enc_obj,  IS_WAIT enc_wait, IS_WAIT que_wait);
// bool audio2rf_encoder_io(u32(*enc_fun)(void *, void *, void *), void *input_func, void *output_func, AUDIO_FORMAT enc_type);
u32 audio2rf_send_data(u8 *data, u32 len);
u32 rf_send_data_api(u8 *data, u16 len, u8 data_type);
void audio2rf_send_register(void *ops);
#endif
