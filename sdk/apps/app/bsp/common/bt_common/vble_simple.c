/***********************************Jieli tech************************************************
  File : vble_simple.c
  date : 2023-11-29
  desp : 该文件是基于简易GATT服务(GATT_simple)在应用层实现的数据收发接口
********************************************************************************************/
#include "vble_simple.h"
#include "bt_ble.h"

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[vble]"
#include "log.h"

#if (TESTE_BLE_EN)

extern const vble_smpl_role_ops ble_slave_ops;
extern const vble_smpl_role_ops ble_master_ops;

#if (SLAVE)
const vble_smpl_role_ops *vble_smpl_ops = &ble_slave_ops;
#elif (MASTER)
const vble_smpl_role_ops *vble_smpl_ops = &ble_master_ops;
#else
#error "VBLE_SIMPLE NO ROLE!"
#endif

void set_vble_smpl_role_ops(const vble_smpl_role_ops *ops)
{
    vble_smpl_ops = ops;
}

void vble_smpl_init(void)
{
    /* extern u8 bt_get_pwr_max_level(void); */
    /* bt_max_pwr_set(10, 5, 8, bt_get_pwr_max_level()); */

    btstack_init();
    vble_smpl_ops->init();
}

void vble_smpl_exit(void)
{
    vble_smpl_ops->exit();
    btstack_exit();
}

int vble_smpl_send_api(u8 *data, u16 len)
{
    return vble_smpl_ops->send(data, len);
}

void vble_smpl_recv_register(void *priv, int (*callback_func)(void *, u8 *, u16))
{
    vble_smpl_ops->recv_cb_register(priv, callback_func);
}

u32 vble_smpl_ioctl(u32 cmd, int arg)
{
    u32 res = 0;
    switch (cmd) {
    case VBLE_SMPL_UPDATE_CONN_INTERVAL:
        res = vble_smpl_ops->update_conn_interval(arg);
        break;
    case VBLE_SMPL_GET_NAME:
        *(char **)arg = vble_smpl_ops->name;
        break;
    case VBLE_SMPL_GET_STATUS:
        *(int *)arg = vble_smpl_ops->get_ble_status();
        break;
    default:
        res = -EINVAL;
    }
    return res;
}

void ble_module_enable(u8 en)
{
    vble_smpl_ops->module_enable(en);
}

void bt_ble_adv_enable(u8 enable)
{
    vble_smpl_ops->adv_enable(enable);
}

void ble_profile_init(void)
{
    vble_smpl_ops->profile_init();
}

void vble_smpl_slave_select(void)
{
#if (SLAVE)
    log_info("bt_ble_slave_var init!\n");
    vble_smpl_ops = &ble_slave_ops;
#endif
}

void vble_smpl_master_select(void)
{
#if (MASTER)
    log_info("bt_ble_master_var init!\n");
    vble_smpl_ops = &ble_master_ops;
#endif
}

void vble_smpl_switch_role(void)
{
#if (MASTER && SLAVE)
    if (vble_smpl_ops == &ble_master_ops) {
        log_info("switch master -> slave\n");
        vble_smpl_slave_select();
    } else {
        log_info("switch slave -> master\n");
        vble_smpl_master_select();
    }
#endif
}
#endif
