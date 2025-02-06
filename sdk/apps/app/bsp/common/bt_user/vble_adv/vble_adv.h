#ifndef __VBLE_ADV_H__
#define __VBLE_ADV_H__

#include "typedef.h"
#include "test_app_config.h"
#include "rf_send_queue.h"
#include "third_party/padvb_intercom/padvb.h"

extern const padvb_lib_ops padvb_tx_op;
extern const padvb_lib_ops padvb_rx_op;


void vble_adv_tx_init(padvb_user_param_t *param);
void vble_adv_tx_uninit(void);
void vble_adv_tx_open(void);
void vble_adv_tx_close(void);
void vble_adv_tx_push_data(u8 *data, u16 len);
void vble_padv_radio_tx_callback(const PADVB_EVENT event);

void vble_adv_rx_init(padvb_user_param_t *param);
void vble_adv_rx_uninit(void);
void vble_adv_rx_open(void);
void vble_adv_rx_close(void);
void vble_adv_rx_pop_data(u8 **data);
void vble_padv_radio_rx_callback(const PADVB_EVENT event);

void vble_adv_init(void);
void vble_adv_uninit(void);
void vble_padv_param_init(void);
int vble_adv_rx_cb_register(void *priv, void *sync_lost_callback_fun, void *pop_data_callback_fun);
// int vble_adv_rx_cb_register(void *priv, void *sync_success_callback_fun, void *sync_lost_callback_fun, void *pop_data_callback_fun);

typedef struct _queue2vbletx_send_mge_ops {
    queue_obj *(*get_send_queue_obj)(void);
    u32(*read_data)(queue_obj *obj, u8 *buf, u32 len);
} queue2vble_tx_send_mge_ops;
void queue2vble_tx_send_register(void *ops);

#define SEND_FRAME_LEN 114 //周期广播发送包长，最大配置不可超过160

extern padvb_user_param_t u_pa_rx AT(.rf_radio_padv);
extern padvb_user_param_t u_pa_tx AT(.rf_radio_padv);
#endif
