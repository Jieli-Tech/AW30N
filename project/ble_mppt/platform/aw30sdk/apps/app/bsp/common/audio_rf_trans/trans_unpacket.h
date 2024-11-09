#ifndef __TRANS_UNPACKET_H__
#define __TRANS_UNPACKET_H__

#include "typedef.h"
#include "audio_rf_mge.h"
#include "trans_packet.h"

typedef enum {
    TUR_DATA_CORRECT = 0,
    TUR_DATA_WRONG = 1,
    TUR_DATA_RECVING = 2,
    TUR_HEAD_RECVING = 3,
} TRANS_UNPAKC_RES;

bool ar_trans_unpack(rev_fsm_mge *ops, void *input, u16 inlen,  u16 *offset);
int unpack_data_deal(rev_fsm_mge *ops, u8 *buff, u16 packet_len);
int register_unpacket_callbck(rev_fsm_mge *ops, void *callback_fun);

s32 packet_cmd_post(void *p_pool, RADIO_PACKET_TYPE cmd, u8 *data, u16 data_len);
u32 packet_cmd_get(void *p_pool, u8 *packet, u8 packet_len);
#endif
