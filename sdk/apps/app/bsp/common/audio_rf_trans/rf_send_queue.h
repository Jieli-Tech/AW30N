#ifndef __RF_SEND_QUEUE_H__
#define __RF_SEND_QUEUE_H__
#include "typedef.h"
#include "circular_buf.h"
#include "hwi.h"
#include "audio_rf_mge.h"

#define RF_SENDER_USE_QUEUE   1

// QS is queue status
#define D_QS_ENABLE   BIT(0)
#define D_QS_CAN_IN   BIT(1)
#define D_QS_CAN_OUT  BIT(2)
#define D_QS_ISR      BIT(3)
#include "trans_packet.h"
typedef struct _queue_obj {
    audio2rf_send_mge_ops *p_send_ops;
    cbuffer_t *cbuf;
    u8 qs_flag;
    u8 index;
} queue_obj;



void kick_rf_queue_isr(queue_obj *obj);
bool regist_rf_queue_isr(queue_obj *obj);
bool unregist_rf_queue_isr(queue_obj *obj);
u32 rf_send_push2queue(queue_obj *obj, u8 *packet_data, u16 packet_len);
u32 rf_queue_init(queue_obj *obj, audio2rf_send_mge_ops *ops, void *p_cbuf);
void rf_queue_uninit(queue_obj *obj, IS_WAIT need_wait);
u32 read_data_from_queue(queue_obj *obj, u8 *buf, u32 len);


#endif
