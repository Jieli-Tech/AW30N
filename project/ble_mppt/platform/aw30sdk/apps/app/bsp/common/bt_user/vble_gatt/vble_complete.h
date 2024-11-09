#ifndef __VBLE_COMPLETE_H__
#define __VBLE_COMPLETE_H__

#include "typedef.h"

// 从机发送主机
#define ATT_SLV2MSTR_HID_IDX        0
#define ATT_SLV2MSTR_RF_RADIO_IDX   1

// 主机发送从机
#define ATT_MSTR2SLV_RF_RADUI_IDX   0


#define VBLE_IOCTL_GET_MASTER_STATUS     1
#define VBLE_IOCTL_GET_SLAVE_STATUS      2

extern void ble_master_init(void);
extern void ble_slave_init(void);
extern int ble_master_data_send(u16 att_handle, u8 *data, u16 len);
extern int ble_slave_data_send(u16 att_handle, u8 *data, u16 len);

// 蓝牙主机相关接口
int vble_master_init_api(void);
int vble_master_send_api(u32 index, u8 *data, u16 len);
int vble_master_recv_cb_register(u32 slv_att_idx, void *priv, void *callback_fun);
void vble_master_receive(u16 att_handle, u8 *recv_data, u16 len);
void vble_master_status_update(u32 status, u8 reason);

// 蓝牙从机相关接口
int vble_slave_init_api(void);
int vble_slave_send_api(u32 index, u8 *data, u16 len);
int vble_slave_recv_cb_register(u32 mstr_att_idx, void *priv, void *callback_fun);
void vble_slave_receive(u16 att_handle, u8 *recv_data, u16 len);
void vble_slave_status_update(u32 status, u8 reason);

u32 vble_ioctl(u32 cmd, int arg);
int vble_uninit(void);
#endif
