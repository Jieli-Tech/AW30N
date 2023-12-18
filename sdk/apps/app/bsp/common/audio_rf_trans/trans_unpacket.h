#ifndef __TRANS_UNPACKET_H__
#define __TRANS_UNPACKET_H__

#include "typedef.h"
#include "audio_rf_mge.h"
#include "trans_packet.h"

bool ar_trans_unpack(rev_fsm_mge *ops, u8 *buff, u16 len, u32 *pcnt);

#endif
