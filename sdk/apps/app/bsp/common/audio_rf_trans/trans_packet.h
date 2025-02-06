#ifndef __TRANS_PACKET_H__
#define __TRANS_PACKET_H__

#include "typedef.h"
#include "audio_rf_mge.h"

typedef struct _audio2rf_send_mge_ops {
    int (*send)(u8 *data, u16 len);
    int (*check_status)(void);
    int (*get_valid_len)(void);
} audio2rf_send_mge_ops;

extern audio2rf_send_mge_ops *g_send_ops;
u16 ar_trans_pack(RADIO_PACKET_TYPE type, u8 *data, u16 data_len, u8 *packet_buf);
u32 audio2rf_send_packet(void *p_queue, RADIO_PACKET_TYPE type, u8 *data, u16 data_len);
#endif
