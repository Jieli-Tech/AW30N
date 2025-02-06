/***********************************Jieli tech************************************************
  File : vble_simple.c
  date : 2023-11-29
  desp : 该文件是基于简易GATT服务(GATT_simple)在应用层实现的数据收发接口
********************************************************************************************/
#include "vble_simple.h"
#include "bt_ble.h"
#include "test_app_config.h"
#include "hid_app_config.h"
#include "spple_app_config.h"

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[vble]"
#include "log.h"

#if (TESTE_BLE_EN)
#if CONFIG_IOT_ENABLE

extern const vble_smpl_role_ops ble_slave_ops;
extern const vble_smpl_role_ops ble_master_ops;

#if (SLAVE)
const vble_smpl_role_ops *vble_smpl_ops = &ble_slave_ops;
#elif (MASTER)
const vble_smpl_role_ops *vble_smpl_ops = &ble_master_ops;
#else
#error "VBLE_SIMPLE NO ROLE!"
#endif

#ifndef WIRELESS_24G_CODE_ID
#define WIRELESS_24G_CODE_ID                    10
#endif

/* static u16 msg_hanle; */
void set_vble_smpl_role_ops(const vble_smpl_role_ops *ops)
{
    vble_smpl_ops = ops;
}

void vble_smpl_init(void)
{
#if (BLE_DUT_TEST || BLE_DUT_API_TEST || CONFIG_BT_MODE != BT_NORMAL)
    //PA测试发射功率不能调太大会导致PA坏掉
#if RF_PA_EN
    bt_max_pwr_set(10, 5, 8, SET_BLE_TX_POWER_LEVEL);
#else
    bt_max_pwr_set(10, 5, 8, bt_get_pwr_max_level());
#endif
    if (BLE_DUT_TEST) {
        user_sele_dut_mode(SET_DUT_MODE);//设置dut mode
    }
    if (BLE_DUT_API_TEST) {
        user_sele_dut_mode(SET_DUT_API_MODE);
    }
#else
    //是否使用2.4G
    /* rf_set_24g_hackable_coded(WIRELESS_24G_CODE_ID); */
#endif

    //底层收发信息打印
#if 0
    void connect_debug_info(void);
    if (!msg_hanle) {
        sys_timer_add(NULL, connect_debug_info, 1000);
    }
#endif

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

void vble_smpl_update_parm_succ_register(void (*callback_func)(int))
{
    vble_smpl_ops->regist_update_parm_succ(callback_func);
}
u32 vble_smpl_ioctl(int cmd, int arg)
{
    u32 res = 0;
    switch (cmd) {
    case VBLE_SMPL_UPDATE_CONN_INTERVAL:
        if (arg == RADIO_WORKING_INTERVAL) {
            res = vble_smpl_ops->update_conn_interval(RADIO_WORKING_INTERVAL, RADIO_WORKING_TIMEOUT);
        } else {
            res = vble_smpl_ops->update_conn_interval(RADIO_STANDBY_INTERVAL, RADIO_STANDBY_TIMEOUT);
        }
        break;
    case VBLE_SMPL_GET_NAME:
        *(char **)arg = vble_smpl_ops->name;
        break;
    case VBLE_SMPL_GET_STATUS:
        *(int *)arg = vble_smpl_ops->get_ble_status();
        break;
    case VBLE_SMPL_GET_CONN_HANDLE:
        *(int *)arg = vble_smpl_ops->conn_handle();
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
#endif
