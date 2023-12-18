/*********************************************************************************************
    *   Filename        : app_comm_ble.c

    *   Description     :

    *   Author          : MRL

    *   Email           : MRL@zh-jieli.com

    *   Last modifiled  : 2021-06-172 14:01

    *   Copyright:(c)JIELI  2011-2021  @ , All Rights Reserved.
*********************************************************************************************/
#include "includes.h"
/* #include "server/server_core.h" */
#include "spple_app_config.h"
#include "app_action.h"
/* #include "os/os_api.h" */
#include "btcontroller_config.h"
#include "bt_controller_include/btctrler_task.h"
/* #include "config/config_transport.h" */
#include "bt_include/avctp_user.h"
#include "bt_include/btstack_task.h"
/* #include "bt_common.h" */
/* #include "le_common.h" */
#include <stdlib.h>
#include "standard_hid.h"
/* #include "rcsp_bluetooth.h" */
/* #include "app_charge.h" */
/* #include "app_power_manage.h" */
/* #include "app_chargestore.h" */
#include "app_comm_bt.h"

#if (TRANS_DATA_SPPLE_EN)
#define LOG_TAG_CONST       COMM_BLE
#define LOG_TAG             "[COMM_BLE]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE
#include "log.h"

#if TCFG_USER_BLE_ENABLE

//默认配置
static const ble_init_cfg_t ble_default_config = {
    .same_address = 0,
    .appearance = 0,
};

extern void ble_standard_dut_test_init(void);
/*************************************************************************************************/
/*!
 *  \brief     协议栈配置，协议栈初始化前调用
 *
 *  \param      [in] 配置参数
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
extern const u8 *bt_get_mac_addr();
void btstack_ble_start_before_init(const ble_init_cfg_t *cfg, int param)
{
    u8 tmp_ble_addr[6];

    if (!cfg) {
        cfg = &ble_default_config;
    }

    if (cfg->same_address) {
        //ble跟edr的地址一样
        memcpy(tmp_ble_addr, bt_get_mac_addr(), 6);
    } else {
        //生成edr对应唯一地址
        bt_make_ble_address(tmp_ble_addr, (void *)bt_get_mac_addr());
    }

    le_controller_set_mac((void *)tmp_ble_addr);

    log_info("---ble's address");
    printf_buf((void *)tmp_ble_addr, 6);

#if CONFIG_BT_GATT_COMMON_ENABLE
    extern void bt_ble_before_start_init(void);
    bt_ble_before_start_init();
#endif

}

/*************************************************************************************************/
/*!
 *  \brief      协议栈初始化后调用
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
void btstack_ble_start_after_init(int param)
{
    if (BT_MODE_IS(BT_BQB)) {
        void ble_bqb_test_thread_init(void);
        ble_bqb_test_thread_init();
    } else {
        extern void bt_ble_init(void);
        bt_ble_init();
    }
}

/*************************************************************************************************/
/*!
 *  \brief    协议栈退出后调用
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
void btstack_ble_exit(int param)
{
    extern void ble_module_enable(u8 en);
    extern void bt_ble_exit(void);
    ble_module_enable(0);
    bt_ble_exit();

    log_info("===btstack_exit\n");
    btstack_exit();
}

/*************************************************************************************************/
/*!
 *  \brief      公共事件消息处理
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
int bt_comm_ble_status_event_handler(struct bt_event *bt)
{
    /* log_info("--------%s: %d",__FUNCTION__, bt->event); */

    switch (bt->event) {
    case BT_STATUS_INIT_OK:
        /*
         * 蓝牙初始化完成
         */
        log_info("STATUS_INIT_OK\n");

#if TCFG_NORMAL_SET_DUT_MODE
        log_info("set dut mode\n");
        ble_standard_dut_test_init();
#else
        btstack_ble_start_after_init(0);
#endif
        break;

    default:
        break;
    }
    return 0;
}

/*************************************************************************************************/
/*!
 *  \brief      hci 事件消息处理
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
int bt_comm_ble_hci_event_handler(struct bt_event *bt)
{
    if (bt->event == HCI_EVENT_VENDOR_REMOTE_TEST) {
        log_info("TEST_BOX:%d", bt->value);
        switch (bt->value) {
        case VENDOR_TEST_DISCONNECTED:
            //TODO
            /* set_remote_test_flag(0); */
            log_info("clear_test_box_flag,note!!!");
            /* cpu_reset();  */
            system_reset(BT_FLAG);
            return 0;
            break;

        case VENDOR_TEST_LEGACY_CONNECTED_BY_BLE:
            break;

        default:
            break;
        }
    }
    return 0;
}

#endif
#endif


