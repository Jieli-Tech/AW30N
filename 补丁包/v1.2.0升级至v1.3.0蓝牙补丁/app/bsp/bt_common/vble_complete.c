/***********************************Jieli tech************************************************
  File : vble_complete.c
  date : 2023-11-29
  desp : 该文件是基于完整GATT服务(GATT_complete)在应用层实现的数据收发接口
********************************************************************************************/
#pragma bss_seg(".vble_com.data.bss")
#pragma data_seg(".vble_com.data")
#pragma const_seg(".vble_com.text.const")
#pragma code_seg(".vble_com.text")
#pragma str_literal_override(".vble_com.text.const")

#include "bt_ble.h"
#include "test_app_config.h"
#include "hid_app_config.h"
#include "spple_app_config.h"
#include "vble_complete.h"
#include "modules/bt/ble_hogp_profile.h"

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[vble]"
#include "log.h"

#if CONFIG_IOT_ENABLE

typedef struct _vble_chn_mge {
    u16 att_handle;
    u16 reserved;
    void *priv;
    void (*callback)(void *, u8 *, u16);
} vble_attchn_mge;

vble_attchn_mge vble_slv2mstr_info[] = {
    {.att_handle = ATT_CHARACTERISTIC_2a4d_01_VALUE_HANDLE, .reserved = 0, .callback = NULL},
    {.att_handle = ATT_CHARACTERISTIC_2a4d_02_VALUE_HANDLE, .reserved = 0, .callback = NULL},
    {.att_handle = ATT_CHARACTERISTIC_2a4d_04_VALUE_HANDLE, .reserved = 0, .callback = NULL},
    {.att_handle = ATT_CHARACTERISTIC_2a4d_05_VALUE_HANDLE, .reserved = 0, .callback = NULL},
    {.att_handle = ATT_CHARACTERISTIC_2a4d_06_VALUE_HANDLE, .reserved = 0, .callback = NULL},
    {.att_handle = ATT_CHARACTERISTIC_2a4d_07_VALUE_HANDLE, .reserved = 0, .callback = NULL},
};

vble_attchn_mge vble_mstr2slv_info[] = {
    {.att_handle = ATT_CHARACTERISTIC_2a4d_03_VALUE_HANDLE, .reserved = 0, .callback = NULL},
};

static volatile u32 master_status;
static volatile u32 slave_status;
/*----------------------------------------------------------------------------*/
/**@brief   蓝牙主机接收处理,由dg_central_event_packet_handler()调用
   @param   att_handle  :数据传输的蓝牙att通道
            recv_data   :数据buff
            len         :数据长度
   @return  无
   @author
   @note    该函数由蓝牙主机协议栈内部调用,函数轮询att通道已注册的回调并调用
*/
/*----------------------------------------------------------------------------*/
void vble_master_receive(u16 att_handle, u8 *recv_data, u16 len)
{
    /* vble_receive(att_handle, recv_data, len); */
    int index;

    for (index = 0; index < ARRAY_SIZE(vble_slv2mstr_info); index++) {
        if (vble_slv2mstr_info[index].att_handle == att_handle) {
            break;
        }
    }
    if (index == ARRAY_SIZE(vble_slv2mstr_info)) {
        /* log_info("att_handle no match!\n"); */
        return;
    }

    /* log_info("vble recv att_handle:0x%x\n", vble_slv2mstr_info[index].att_handle); */
    if (NULL != vble_slv2mstr_info[index].callback) {
        void *priv = vble_slv2mstr_info[index].priv;
        vble_slv2mstr_info[index].callback(priv, recv_data, len);
    }
}

/*----------------------------------------------------------------------------*/
/**@brief   蓝牙从机接收处理,由hogp_att_write_callback()调用
   @param   att_handle  :数据传输的蓝牙att通道
            recv_data   :数据buff
            len         :数据长度
   @return  无
   @author
   @note    该函数由蓝牙从机协议栈内部调用,函数轮询att通道已注册的回调并调用
*/
/*----------------------------------------------------------------------------*/
void vble_slave_receive(u16 att_handle, u8 *recv_data, u16 len)
{
    /* vble_receive(att_handle, recv_data, len); */
    int index;

    for (index = 0; index < ARRAY_SIZE(vble_mstr2slv_info); index++) {
        if (vble_mstr2slv_info[index].att_handle == att_handle) {
            break;
        }
    }
    if (index == ARRAY_SIZE(vble_mstr2slv_info)) {
        /* log_info("att_handle no match!\n"); */
        return;
    }

    log_info("vble recv att_handle:0x%x\n", vble_mstr2slv_info[index].att_handle);
    if (NULL != vble_mstr2slv_info[index].callback) {
        void *priv = vble_mstr2slv_info[index].priv;
        vble_mstr2slv_info[index].callback(priv, recv_data, len);
    }
}

/*----------------------------------------------------------------------------*/
/**@brief   注册蓝牙主机接收从机att发送通道对应的接收回调函数
   @param   slv_att_idx     :蓝牙从机发送的att通道号
            priv            :私有参数
            callback_fun    :回调函数
   @return  0:成功  -1:失败
   @author
   @note    从机通过该att通道发送数据,主机会调用该注册的回调函数
*/
/*----------------------------------------------------------------------------*/
int vble_master_recv_cb_register(u32 slv_att_idx, void *priv, void *callback_fun)
{
    if (slv_att_idx >= ARRAY_SIZE(vble_slv2mstr_info)) {
        log_error("slv_att_idx is bigger than Array_size!");
        return -1;
    }
    local_irq_disable();
    vble_slv2mstr_info[slv_att_idx].priv = priv;
    vble_slv2mstr_info[slv_att_idx].callback = callback_fun;
    local_irq_enable();
    return 0;
}
/*----------------------------------------------------------------------------*/
/**@brief   注册蓝牙从机att通道对应的接收回调函数
   @param   mstr_att_idx    :蓝牙主机发送的att通道号
            priv            :私有参数
            callback_fun    :回调函数
   @return  0:成功  -1:失败
   @author
   @note    主机通过该att通道发送数据,从机会调用该注册的回调函数
*/
/*----------------------------------------------------------------------------*/
int vble_slave_recv_cb_register(u32 mstr_att_idx, void *priv, void *callback_fun)
{
    if (mstr_att_idx >= ARRAY_SIZE(vble_mstr2slv_info)) {
        log_error("mstr_att_idx is bigger than Array_size!");
        return -1;
    }
    local_irq_disable();
    vble_mstr2slv_info[mstr_att_idx].priv = priv;
    vble_mstr2slv_info[mstr_att_idx].callback = callback_fun;
    local_irq_enable();
    return 0;
}
/*----------------------------------------------------------------------------*/
/**@brief   蓝牙从机发送数据
   @param   slv_att_idx:att通道号
            data    :数据buff
            len     :数据长度
   @return  0:成功  非0:失败
   @author
   @note    从机选择att通道进行数据发送,主机接收并回调att通道已注册的回调
*/
/*----------------------------------------------------------------------------*/
int vble_slave_send_api(u32 slv_att_idx, u8 *data, u16 len)
{
    if (slv_att_idx >= ARRAY_SIZE(vble_slv2mstr_info)) {
        log_error("slv_att_idx is bigger than Array_size!");
        return -1;
    }
    /* log_info("slv att_send:0x%x\n", vble_slv2mstr_info[slv_att_idx].att_handle); */
    return ble_slave_data_send(vble_slv2mstr_info[slv_att_idx].att_handle, data, len);
}
/*----------------------------------------------------------------------------*/
/**@brief   蓝牙主机发送数据
   @param   mstr_att_idx:att通道号
            data    :数据buff
            len     :数据长度
   @return  0:成功  非0:失败
   @author
   @note    主机选择att通道进行数据发送,从机接收并回调att通道已注册的回调
*/
/*----------------------------------------------------------------------------*/
int vble_master_send_api(u32 mstr_att_idx, u8 *data, u16 len)
{
    if (mstr_att_idx >= ARRAY_SIZE(vble_mstr2slv_info)) {
        log_error("mstr_att_idx is bigger than Array_size!");
        return -1;
    }
    log_info("mstr att_send:0x%x\n", vble_mstr2slv_info[mstr_att_idx].att_handle);
    return ble_master_data_send(vble_mstr2slv_info[mstr_att_idx].att_handle, data, len);
}

/*----------------------------------------------------------------------------*/
/**@brief   蓝牙主机更新连接状态
   @param   status  :蓝牙状态
            reason  :附加原因
   @return
   @author
   @note    蓝牙通过该接口设置主机连接状态
*/
/*----------------------------------------------------------------------------*/
void vble_master_status_update(u32 status, u8 reason)
{
    local_irq_disable();
    master_status = status;
    local_irq_enable();

}

/*----------------------------------------------------------------------------*/
/**@brief   蓝牙从机更新连接状态
   @param   status  :蓝牙状态
            reason  :附加原因
   @return
   @author
   @note    蓝牙通过该接口设置从机连接状态
*/
/*----------------------------------------------------------------------------*/
void vble_slave_status_update(u32 status, u8 reason)
{
    local_irq_disable();
    slave_status = status;
    local_irq_enable();
}
/*----------------------------------------------------------------------------*/
/**@brief   蓝牙主机初始化
   @param   无
   @return  0:成功
   @author
   @note    int vble_master_init_api(void)
*/
/*----------------------------------------------------------------------------*/
int vble_master_init_api(void)
{
    ble_clock_init();
    ble_master_init();
    return 0;
}

/*----------------------------------------------------------------------------*/
/**@brief   蓝牙ioctl
   @param   无
   @return  0:成功
   @author
   @note    u32 vble_ioctl(u32 cmd, u32 arg)

*/
/*----------------------------------------------------------------------------*/
u32 vble_ioctl(u32 cmd, int arg)
{
    u32 res = 0;
    switch (cmd) {
    case VBLE_IOCTL_GET_MASTER_STATUS:
        *(int *)arg = master_status;
        break;
    case VBLE_IOCTL_GET_SLAVE_STATUS:
        *(int *)arg = slave_status;
        break;

    default:
        res = -EINVAL;
    }
    return res;
}
/*----------------------------------------------------------------------------*/
/**@brief   蓝牙从机初始化
   @param   无
   @return  0:成功
   @author
   @note    int vble_slave_init_api(void)
*/
/*----------------------------------------------------------------------------*/
int vble_slave_init_api(void)
{
    ble_clock_init();
    ble_slave_init();
    return 0;
}

#if 0
//TODO 接口未完善
int vble_uninit(void)
{
    ble_module_enable(0);
    /* btstack irq uninit */
    return 0;
}
#endif
#endif


