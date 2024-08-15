#ifndef BTSTACK_TASK_H
#define BTSTACK_TASK_H

int btstack_init();
int btstack_exit();
void ble_bqb_test_thread_init(void);
int bt_write_cbuf(const char *name, int type, int argc, int *argv);
void df_aoa_broadcast_test_open(void);
void df_aoa_tx_connected_test_open(void);
void aes128_test();
void aes_ccm_test(void);
#endif
