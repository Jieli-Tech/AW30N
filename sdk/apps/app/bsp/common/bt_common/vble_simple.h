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

void vble_smpl_init(void);
void vble_smpl_exit(void);
int vble_smpl_send_api(u8 *data, u16 len);
void vble_smpl_recv_register(void *priv, int (*callback_func)(void *, u8 *, u16));
u32 vble_smpl_ioctl(u32 cmd, int arg);
void ble_module_enable(u8 en);
void bt_ble_adv_enable(u8 enable);
void ble_profile_init(void);

void vble_smpl_slave_select(void);
void vble_smpl_master_select(void);
void vble_smpl_switch_role(void);

#endif
