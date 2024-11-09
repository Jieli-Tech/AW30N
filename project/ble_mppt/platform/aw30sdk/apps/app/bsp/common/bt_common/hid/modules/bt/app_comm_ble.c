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
#include "hid_app_config.h"
#include "app_action.h"
/* #include "os/os_api.h" */
#include "btcontroller_config.h"
#include "bt_controller_include/btctrler_task.h"
/* #include "config/config_transport.h" */
#include "bt_include/avctp_user.h"
#include "bt_include/btstack_task.h"
#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".ble_app_bss")
#pragma data_seg(".ble_app_data")
#pragma const_seg(".ble_app_text_const")
#pragma code_seg(".ble_app_text")
#endif

#if (TRANS_DATA_HID_EN)
/* #include "bt_common.h" */
#include "edr_hid_user.h"
/* #include "le_common.h" */
#include <stdlib.h>
#include "standard_hid.h"
/* #include "rcsp_bluetooth.h" */
/* #include "app_charge.h" */
/* #include "app_power_manage.h" */
/* #include "app_chargestore.h" */
#include "app_comm_bt.h"

#define LOG_TAG_CONST       COMM_BLE
#define LOG_TAG             "[COMM_BLE]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE
#include "log.h"
/* #include "debug.h" */

#if TCFG_USER_BLE_ENABLE
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
extern u16 app_power_get_vbat(void);
void btstack_ble_start_before_init(const ble_init_cfg_t *cfg, int param)
{
    u8 tmp_ble_addr[6];

    if (!cfg) {
        log_error("cfg is null!!!");
        ASSERT(0);
    }

    if (cfg->same_address) {
        //ble跟edr的地址一样
        memcpy(tmp_ble_addr, bt_get_mac_addr(), 6);
    } else {
        //生成edr对应唯一地址
        bt_make_ble_address(tmp_ble_addr, (void *)bt_get_mac_addr());
    }

    le_controller_set_mac((void *)tmp_ble_addr);

    extern void le_hogp_set_icon(u16 class_type);
    extern void le_hogp_set_ReportMap(u8 * map, u16 size);
    extern void le_hogp_regiest_get_battery(u8(*get_battery_cbk)(void));
    le_hogp_set_icon(cfg->appearance);
    le_hogp_set_ReportMap(cfg->report_map, cfg->report_map_size);
    le_hogp_regiest_get_battery(app_power_get_vbat);

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
    log_info("--------%s: %d", __FUNCTION__, bt->event);

    switch (bt->event) {
    case BT_STATUS_INIT_OK:
        /*
         * 蓝牙初始化完成
         */
        log_info("STATUS_INIT_OK\n");

#if BLE_DUT_TEST
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
            log_info("clear_test_box_flag");
            /* cpu_reset(); */
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

