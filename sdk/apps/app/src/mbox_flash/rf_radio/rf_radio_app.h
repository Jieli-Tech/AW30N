#ifndef __RF_RADIO_APP_H__
#define __RF_RADIO_APP_H__

#include "typedef.h"
#include "app.h"

extern void bt_init_api(void);
extern void app_ble_recv_callback_register(int (*callback_func)(u8 *, u16));
extern u16 rf_radio_key_msg_filter(u8 key_status, u8 key_num, u8 key_type);
extern bool rf_recv_data_check(void *rf_packet, u16 packet_len);
void rf_radio_app(void);

#endif

