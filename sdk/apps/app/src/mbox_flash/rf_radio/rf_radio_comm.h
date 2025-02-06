#ifndef __RF_RADIO_COMM_H__
#define __RF_RADIO_COMM_H__

#include "typedef.h"
#include "decoder_mge.h"
#include "sound_mge.h"
#include "circular_buf.h"
#include "audio_rf_mge.h"

typedef struct __rfr_dec {
    dec_obj *obj;
    sound_stream_obj stream;
    cbuffer_t icbuf;
} rfr_dec;

dec_obj *rfr_decode_start_phy(RF_RADIO_ENC_HEAD *p_enc_head, rfr_dec *p_rfr_dec, void *ibuf, u32 ibuf_len, void *kick, rev_fsm_mge *p_packet);
void rfr_decode_stop_phy(rfr_dec *p_rfr_dec, rev_fsm_mge *p_packet);

#endif
