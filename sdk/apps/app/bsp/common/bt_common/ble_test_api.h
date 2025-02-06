#ifndef  _BLE_TEST_API_H__
#define  _BLE_TEST_API_H__
#include "typedef.h"

void ble_dut_tx_fre_api(u8 ch);
void ble_dut_rx_fre_api(u8 ch);
int ble_dut_test_end(void);
void ble_standard_dut_test_init(void);
void ble_standard_dut_test_close(void);
void ble_dut_mode_key_handle(u8 type, u8 key_val);
#endif  //_BLE_TEST_API_H__

