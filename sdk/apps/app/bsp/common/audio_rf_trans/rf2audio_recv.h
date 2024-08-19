#ifndef __RF_RECEIVER_H__
#define __RF_RECEIVER_H__
#include "typedef.h"
#include "decoder_mge.h"
#include "audio_rf_mge.h"

typedef enum {
    STREAM_TO_DAC   = 0,
    STREAM_TO_USB   = 1,
} STREAM_CHANNEL;

void rf2audio_receiver_kick_decoder(void *p_stream_in, void *psound);
u32 rf2audio_receiver_write_data(dec_obj *p_recv_dec_obj, u8 *data, u32 data_len);
u32 rf2audio_receiver_read_data(u8 *data, u32 data_len);
dec_obj *rf2audio_decoder_start(RF_RADIO_ENC_HEAD *p_enc_head, sound_stream_obj *p_rf_stream, u32 output_sr);
bool rf2audio_decoder_stop(dec_obj *p_recv_dec_obj, bool(*unregist_func)(void *));
// void rf_receiver_init(void);
void set_usb_mic_info(void *source);
#endif
