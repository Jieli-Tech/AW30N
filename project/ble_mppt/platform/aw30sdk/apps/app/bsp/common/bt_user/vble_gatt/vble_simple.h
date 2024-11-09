#ifndef __VBLE_SIMPLE_H__
#define __VBLE_SIMPLE_H__

#include "typedef.h"
#include "bt_ble.h"
#include "test_app_config.h"

enum {
    VBLE_SMPL_UPDATE_CONN_INTERVAL = 0,
    VBLE_SMPL_GET_NAME,
    VBLE_SMPL_GET_STATUS,
};


//interval >= 4000配置为私有连接参数,interval=4000,实际连接间隔为4000us
//interval <  600 配置为标准连接参数,interval=30,实际连接间隔为30*1.25ms=37.5ms
#define RADIO_WORKING_INTERVAL      2000
#define RADIO_STANDBY_INTERVAL      50000

#define RADIO_WORKING_TIMEOUT      200
#define RADIO_STANDBY_TIMEOUT      1000

void vble_smpl_init(void);
void vble_smpl_exit(void);
int vble_smpl_send_api(u8 *data, u16 len);
void vble_smpl_recv_register(void *priv, int (*callback_func)(void *, u8 *, u16));
u32 vble_smpl_ioctl(int cmd, int arg);
void ble_module_enable(u8 en);
void bt_ble_adv_enable(u8 enable);
void ble_profile_init(void);

void vble_smpl_slave_select(void);
void vble_smpl_master_select(void);
void vble_smpl_switch_role(void);




#endif
