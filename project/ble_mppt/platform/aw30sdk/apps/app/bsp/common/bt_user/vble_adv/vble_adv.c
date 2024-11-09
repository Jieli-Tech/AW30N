
#pragma bss_seg(".vble_adv.data.bss")
#pragma data_seg(".vble_adv.data")
#pragma const_seg(".vble_adv.text.const")
#pragma code_seg(".vble_adv.text")
#pragma str_literal_override(".vble_com.text.const")

#include "vble_adv.h"
#include "bt_ble.h"

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[vble_adv]"
#include "log.h"
/* #include "third_party/padvb_intercom/padvb.h" */
const padvb_lib_ops *vble_adv_tx_ops = &padvb_tx_op;
const padvb_lib_ops *vble_adv_rx_ops = &padvb_rx_op;
void vble_adv_tx_init(padvb_user_param_t *param)
{
    vble_adv_tx_ops->padvb_init(param);
}
void vble_adv_tx_uninit(void)
{
    vble_adv_tx_ops->padvb_uninit();
}
void vble_adv_tx_open(void)
{
    vble_adv_tx_ops->padvb_open();
}
void vble_adv_tx_close(void)
{
    vble_adv_tx_ops->padvb_close();
}
void vble_adv_tx_push_data(u8 *data, u16 len)
{
    vble_adv_tx_ops->padvb_push_data(data, len);
}

void vble_adv_rx_init(padvb_user_param_t *param)
{
    vble_adv_rx_ops->padvb_init(param);
}
void vble_adv_rx_uninit(void)
{
    vble_adv_rx_ops->padvb_uninit();
}
void vble_adv_rx_open(void)
{
    vble_adv_rx_ops->padvb_open();
}
void vble_adv_rx_close(void)
{
    vble_adv_rx_ops->padvb_close();
}
void vble_adv_rx_pop_data(u8 **data)
{
    vble_adv_rx_ops->padvb_pop_data(data);
}

void vble_adv_init(void)
{
    btstack_init();
}
void vble_adv_uninit(void)
{
    btstack_exit();

}

