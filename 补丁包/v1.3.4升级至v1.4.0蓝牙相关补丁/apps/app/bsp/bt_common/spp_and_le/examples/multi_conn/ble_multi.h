// binary representation
// attribute size in bytes (16), flags(16), handle (16), uuid (16/128), value(...)

#ifndef _BLE_MUTIL_H
#define _BLE_MUTIL_H

#include <stdint.h>
#include "app_config.h"

//ATT发送的包长,    note: 23 <=need >= MTU
#define ATT_LOCAL_MTU_SIZE        (64)

//ATT缓存的buffer支持缓存数据包个数
#define ATT_PACKET_NUMS_MAX       (2)

//ATT缓存的buffer大小,  note: need >= 23,可修改
#define ATT_SEND_CBUF_SIZE        (ATT_PACKET_NUMS_MAX * (ATT_PACKET_HEAD_SIZE + ATT_LOCAL_MTU_SIZE))


void ble_module_enable(uint8_t en);
void ble_multi_key_test(uint8_t key_type, uint8_t key_value);
void multi_server_init(void);
void multi_server_exit(void);
void multi_client_init(void);
void multi_client_exit(void);
int multi_client_clear_pair(void);
int multi_server_clear_pair(void);
void multi_state_idle_set_active(uint8_t active);
void multi_client_key_test(uint8_t key_type, uint8_t key_value);
void multi_server_key_test(uint8_t key_type, uint8_t key_value);
#endif
