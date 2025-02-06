#ifndef __VBLE_SIMPLE_H__
#define __VBLE_SIMPLE_H__

#include "typedef.h"
#include "bt_ble.h"
#include "test_app_config.h"

enum {
    VBLE_SMPL_UPDATE_CONN_INTERVAL = 0,
    VBLE_SMPL_GET_NAME,
    VBLE_SMPL_GET_STATUS,
    VBLE_SMPL_GET_CONN_HANDLE,
};

//interval <  600 配置为标准连接参数,interval=30,实际连接间隔为30*1.25ms=37.5ms

//包长137的安全距离 以下参数为发布版本参数，不建议修改。如果包长有超过137的需求，需要联系蓝牙同事更新
#if (CONFIG_BLE_PHY_SET == CONFIG_SET_CODED_S2_PHY)
#if defined(FULL_DUPLEX_RADIO) && (FULL_DUPLEX_RADIO == 1)
#define RADIO_WORKING_INTERVAL      6500
#else
#define RADIO_WORKING_INTERVAL      5000
#endif

#else
#if defined(FULL_DUPLEX_RADIO) && (FULL_DUPLEX_RADIO == 1)
#define RADIO_WORKING_INTERVAL      3000
#else
#define RADIO_WORKING_INTERVAL      2500
#endif
#endif

#define RADIO_STANDBY_INTERVAL      50000

#define RADIO_WORKING_TIMEOUT      200
#define RADIO_STANDBY_TIMEOUT      1000

void vble_smpl_init(void);
void vble_smpl_exit(void);
int vble_smpl_send_api(u8 *data, u16 len);
void vble_smpl_recv_register(void *priv, int (*callback_func)(void *, u8 *, u16));
void vble_smpl_update_parm_succ_register(void (*callback_func)(int));
u32 vble_smpl_ioctl(int cmd, int arg);
void ble_module_enable(u8 en);
void bt_ble_adv_enable(u8 enable);
void ble_profile_init(void);

void vble_smpl_slave_select(void);
void vble_smpl_master_select(void);
void vble_smpl_switch_role(void);




#endif
