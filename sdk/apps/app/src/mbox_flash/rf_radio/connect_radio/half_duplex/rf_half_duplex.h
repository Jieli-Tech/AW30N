#ifndef __RF_RADIO_HALF_DUPLEX_H__
#define __RF_RADIO_HALF_DUPLEX_H__


#include "rf_radio_app.h"

typedef struct __apply_bt_info_struct {
    u32 bt_interval;
    u32 bt_latency;
    u32 bt_timeout;
} apply_bt_info_struct;

extern rev_fsm_mge g_half_packet;
extern enum RFR_HALF_MODE g_rfr_half_mode;

u32 get_curr_true_interval(void);
void rfr_half_bt2applyinit(void);

void rfr_half_unpacket_cmd_pool_init(void);

void rf_radio_half_duplex_loop(void);   //半双工对接机的应用模式
void rrapp_sending(int active_msg);     //半双工对接机的发送模式
void rrapp_receiving(int active_msg);   //半双工对讲机的接收模式

#endif
