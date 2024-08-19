#ifndef __RF_SEND_QUEUE_H__
#define __RF_SEND_QUEUE_H__
#include "typedef.h"
#include "circular_buf.h"
#include "hwi.h"
#include "audio_rf_mge.h"

#define RF_SENDER_USE_QUEUE   1

void kick_rf_queue_isr(void);

// bool rf_send_push2queue(RADIO_PACKET_TYPE type, u8 *data, u16 data_len, u8 *packet_buf);
u32 rf_send_push2queue(u8 *packet_data, u16 packet_len);
void rf_send_soft_isr_init(void *p_cbuf, u8 use_queue_isr);
void rf_send_soft_isr_uninit(void);
u32 read_data_from_queue(u8 *buf, u32 len);
#endif
