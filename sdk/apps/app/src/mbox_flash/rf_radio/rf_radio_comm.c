
#include "rf_radio_comm.h"

#include "audio_rf_mge.h"
#include "rf2audio_recv.h"

#include "audio_dac_api.h"
#include "typedef.h"
#include "app_config.h"

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[rf_radio_comm]"
#include "log.h"

dec_obj *rfr_decode_start_phy(RF_RADIO_ENC_HEAD *p_enc_head, rfr_dec *p_rfr_dec, void *ibuf, u32 ibuf_len, void *kick, rev_fsm_mge *p_packet)
{
    log_info("rfr_decode_start_phy\n");
    if (NULL != p_rfr_dec->obj) {
        log_error("rf dev_obj not null ");
    }
    dac_init(SR_DEFAULT, 0);
    cbuf_init(&p_rfr_dec->icbuf, ibuf, ibuf_len);
    memset(&p_rfr_dec->stream, 0, sizeof(sound_stream_obj));
    p_rfr_dec->stream.p_ibuf =  &p_rfr_dec->icbuf;
    dec_obj *p_dec_obj = rf2audio_decoder_start(p_enc_head, &p_rfr_dec->stream, SR_DEFAULT);
    if (NULL == p_dec_obj) {
        memset(&p_rfr_dec->stream, 0, sizeof(sound_stream_obj));
    } else {
        p_rfr_dec->obj = p_dec_obj;
        regist_dac_channel(NULL, &p_dec_obj->sound, kick);
        p_packet->dec_obj = p_dec_obj;
    }
    return p_dec_obj;

}



void rfr_decode_stop_phy(rfr_dec *p_rfr_dec, rev_fsm_mge *p_packet)
{
    log_info("rfr_decode_stop_phy\n");
    if (NULL != p_rfr_dec->obj) {
        rf2audio_decoder_stop(p_rfr_dec->obj, unregist_dac_channel);
        p_rfr_dec->obj = NULL;
        p_rfr_dec->stream.p_ibuf = NULL;
        p_packet->dec_obj = NULL;
    }
    dac_off_api();

}
