/*********************************************************************************************
    *   Filename        : ble_multi.c

    *   Description     :

    *   Author          :

    *   Email           :

    *   Last modifiled  : 2023-10-05 10:09

    *   Copyright:(c)JIELI  2023-2031  @ , All Rights Reserved.
*********************************************************************************************/
#include <stdlib.h>
#include "msg.h"
#include "bt_ble.h"
#include "includes.h"
#include "spple_app_config.h"
#include "app_action.h"
#include "btcontroller_config.h"
#include "bt_controller_include/btctrler_task.h"
#include "bt_include/avctp_user.h"
#include "bt_include/btstack_task.h"
#include "app_power_mg.h"
#include "app_comm_bt.h"
#include "sys_timer.h"
#include "gpio.h"
#include "app_modules.h"
#include "le_gatt_common.h"
#include "ble_multi.h"
#include "user_cfg.h"
#include "btcontroller_modules.h"
#if RCSP_BTMATE_EN
#include "rcsp_bluetooth.h"
#endif

#if (TRANS_DATA_SPPLE_EN && CONFIG_APP_MULTI)

#ifdef CONFIG_DEBUG_ENABLE
#define log_info(x, ...)  printf("[MUL]" x " ", ## __VA_ARGS__)
#define log_info_hexdump  put_buf

#else
#define log_info(...)
#define log_info_hexdump(...)
#endif

static char gap_device_name[BT_NAME_LEN_MAX] = "jl_ble_test";
static u8 gap_device_name_len = 0;     //名字长度，不包含结束符
static const char ble_ext_name[] = "(BLE)";
//------------------------------------------------------
//------------------------------------------------------
//输入passkey 加密
#define PASSKEY_ENABLE                     0

static const sm_cfg_t multi_sm_init_config = {
    .master_security_auto_req = 1,
    .master_set_wait_security = 1,
    .slave_security_auto_req = 0,
    .slave_set_wait_security = 1,

#if PASSKEY_ENABLE
    .io_capabilities = IO_CAPABILITY_DISPLAY_ONLY,
#else
    .io_capabilities = IO_CAPABILITY_NO_INPUT_NO_OUTPUT,
#endif

    .authentication_req_flags = SM_AUTHREQ_BONDING | SM_AUTHREQ_MITM_PROTECTION,
    .min_key_size = 7,
    .max_key_size = 16,
    .sm_cb_packet_handler = NULL,
};

extern const gatt_server_cfg_t multi_server_init_cfg;
extern const gatt_client_cfg_t multi_client_init_cfg;

//gatt 控制块初始化
static gatt_ctrl_t multi_gatt_control_block = {
    //public
    .mtu_size = ATT_LOCAL_MTU_SIZE,
    .cbuffer_size = ATT_SEND_CBUF_SIZE,
    .multi_dev_flag	= 1,

    //config
#if CONFIG_BT_GATT_SERVER_NUM
    .server_config = &multi_server_init_cfg,
#else
    .server_config = NULL,
#endif

#if CONFIG_BT_GATT_CLIENT_NUM
    .client_config = &multi_client_init_cfg,
#else
    .client_config = NULL,
#endif

#if CONFIG_BT_SM_SUPPORT_ENABLE
    .sm_config = &multi_sm_init_config,
#else
    .sm_config = NULL,
#endif
    //cbk,event handle
    .hci_cb_packet_handler = NULL,
};

void ble_multi_key_test(uint8_t key_type, uint8_t key_value)
{
#if CONFIG_BT_GATT_CLIENT_NUM
    /* multi_client_key_test(key_type, key_value); */
#endif

#if CONFIG_BT_GATT_SERVER_NUM
    /* multi_server_key_test(key_type, key_value); */
#endif
}

/*************************************************************************************************/
/*!
 *  \brief      ble 模块使能
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
void ble_module_enable(uint8_t en)
{
    ble_comm_module_enable(en);
}

/*************************************************************************************************/
/*!
 *  \brief      协议栈开始前初始化
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
void bt_ble_before_start_init(void)
{
    log_info("%s", __FUNCTION__);
    ble_comm_init(&multi_gatt_control_block);
}

/*************************************************************************************************/
/*!
 *  \brief      模块初始化
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
void bt_ble_init(void)
{
    log_info("%s\n", __FUNCTION__);
    log_info("ble_file: %s", __FILE__);

    char *name_p;
    u8 ext_name_len = sizeof(ble_ext_name) - 1;

    /* rf_set_24g_hackable_coded(CFG_RF_24G_CODE_ID); */
    /* ble_rf_vendor_fixed_channel(36, 3); //通信信道，发送接收都要一致 */

    name_p = bt_get_local_name();
    gap_device_name_len = strlen(name_p);
    if (gap_device_name_len > BT_NAME_LEN_MAX - ext_name_len) {
        gap_device_name_len = BT_NAME_LEN_MAX - ext_name_len;
    }

    //增加后缀，区分名字
    memcpy(gap_device_name, name_p, gap_device_name_len);
    memcpy(&gap_device_name[gap_device_name_len], "(BLE)", ext_name_len);
    gap_device_name_len += ext_name_len;

    log_info("ble name(%d): %s \n", gap_device_name_len, gap_device_name);

#if CONFIG_BT_GATT_SERVER_NUM
    multi_server_init();
#if CFG_USE_24G_CODE_ID_ADV
    rf_set_adv_24g_hackable_coded(CFG_RF_24G_CODE_ID_ADV);//设置为2.4g模式
    log_info("======slave 24g mode\n");
#endif
#endif

#if CONFIG_BT_GATT_CLIENT_NUM
    // 默认scan不开低功耗
    /* multi_state_idle_set_active(1); */
    multi_client_init();

#if CFG_USE_24G_CODE_ID_SCAN
    rf_set_scan_24g_hackable_coded(CFG_RF_24G_CODE_ID_SCAN);//设置为2.4g模式
    log_info("======master 24g mode\n");
#endif

#endif
    ble_module_enable(1);
}


/*************************************************************************************************/
/*!
 *  \brief      模块退出
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
void bt_ble_exit(void)
{
    log_info("%s\n", __FUNCTION__);
    ble_module_enable(0);

    ble_comm_exit();

#if CONFIG_BT_GATT_SERVER_NUM
    multi_server_exit();
#endif

#if CONFIG_BT_GATT_CLIENT_NUM
    multi_client_exit();
#endif

}
#endif



