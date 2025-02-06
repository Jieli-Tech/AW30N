/*********************************************************************************************
    *   Filename        : app_multi_conn.c

    *   Description     :

    *   Author          :

    *   Email           :

    *   Last modifiled  : 2023-10-05 10:09

    *   Copyright:(c)JIELI  2023-2031  @ , All Rights Reserved.
*********************************************************************************************/
#include "msg.h"
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
#include <stdlib.h>
#include "app_modules.h"
#include "le_gatt_common.h"
#include "user_cfg.h"
#include "ble_multi.h"
#if RCSP_BTMATE_EN
#include "rcsp_bluetooth.h"
#endif

#define LOG_TAG_CONST       MULTI_CONN
#define LOG_TAG             "[MULTI_CONN]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE
#include "log.h"


#if (TRANS_DATA_SPPLE_EN && CONFIG_APP_MULTI)

static volatile uint8_t multi_is_active = 0;
/*************************************************************************************************/
/*!
 *  \brief      设置低功耗状态
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
void multi_state_idle_set_active(uint8_t active)
{
    multi_is_active = active;
}

/*************************************************************************************************/
/*!
 *  \brief      进入软关机
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
static void multi_set_soft_poweroff(void)
{
    log_info("set_soft_poweroff\n");
    multi_is_active = 1;
    //必须先主动断开蓝牙链路,否则要等链路超时断开

#if TCFG_USER_BLE_ENABLE
    btstack_ble_exit(0);
#endif

    power_set_soft_poweroff();
}

/*************************************************************************************************/
/*!
 *  \brief      app  start
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
static void multi_app_start()
{
    static u32 i = 0;

    log_info("=======================================");
    log_info("-----------multi_conn demo-------------");
    log_info("=======================================");
    log_info("app_file: %s", __FILE__);

//有蓝牙
#if TCFG_USER_BLE_ENABLE
    /* clk_set("sys", (48 * 1000000L)); */
    SFR(JL_CLOCK->PRP_CON1,  0, 3, 1);
    SFR(JL_CLOCK->PRP_CON1, 26, 1, 1);
    SFR(JL_CLOCK->PRP_CON2, 0, 2, 1);
    SFR(JL_CLOCK->PRP_CON2, 2, 2, 2);
    SFR(JL_CLOCK->PRP_CON2, 4, 2, 2);
    SFR(JL_CLOCK->PRP_CON2, 8, 1, 1);
    asm("csync");

    bt_pll_para(48000000, 48000000, 0, 0);
    btstack_ble_start_before_init(NULL, 0);

    btstack_init();
#endif

    while (1) {

        i++;
#if CONFIG_BLE_CONNECT_SLOT
        //调小间隔后,缩小wdt time
        if (i > 0xfff) {
#else
        if (i > 0xffff) {
#endif
            putchar('~');
            i = 0;
            wdt_clear();
        }
    }
}

/*************************************************************************************************/
/*!
 *  \brief      app 状态机处理
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
static int multi_state_machine(struct application *app, enum app_state state, struct intent *it)
{
    switch (state) {
    case APP_STA_CREATE:
        break;

    case APP_STA_START:
        if (!it) {
            break;
        }
        switch (it->action) {
        case ACTION_MULTI_MAIN:
            multi_app_start();
            break;
        }
        break;

    case APP_STA_PAUSE:
        break;

    case APP_STA_RESUME:
        break;

    case APP_STA_STOP:
        break;

    case APP_STA_DESTROY:
        log_info("APP_STA_DESTROY\n");
        break;
    }

    return 0;
}

static int multi_bt_hci_event_handler(struct bt_event *bt)
{
    //对应原来的蓝牙连接上断开处理函数  ,bt->value=reason
    log_info("----%s reason %x %x", __FUNCTION__, bt->event, bt->value);
    bt_comm_ble_hci_event_handler(bt);
    return 0;
}

static int multi_bt_connction_status_event_handler(struct bt_event *bt)
{
    log_info("----%s %d", __FUNCTION__, bt->event);
    bt_comm_ble_status_event_handler(bt);
    return 0;
}

/*************************************************************************************************/
/*!
 *  \brief      修改2.4G CODED码
 *  \param      [in] coded         设置coded码为32bits.
 *  \param      [in] channel       设置广播(GATT_ROLE_SERVER) or 扫描(GATT_ROLE_CLIENT)
 *
 *  \note       在初始化完成后任意非连接时刻修改CODED码
 */
/*************************************************************************************************/
#if CFG_USE_24G_CODE_ID_ADV || CFG_USE_24G_CODE_ID_SCAN
void multi_set_conn_24g_coded(uint32_t coded, uint8_t channel)
{
    if (channel == GATT_ROLE_CLIENT) {
        ble_gatt_client_module_enable(0);
        rf_set_scan_24g_hackable_coded(coded);
        ble_gatt_client_module_enable(1);
    } else {
        ble_gatt_server_module_enable(0);
        rf_set_adv_24g_hackable_coded(coded);
        ble_gatt_server_module_enable(1);
    }
}
#endif

/*************************************************************************************************/
/*!
 *  \brief      按键事件处理
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
static void multi_key_event_handler(struct sys_event *event)
{
    if (event->arg == (void *)DEVICE_EVENT_FROM_KEY) {
        uint8_t event_type = 0;
        uint8_t key_value = 0;

        event_type = event->u.key.event;
        key_value = event->u.key.value;
        log_info("app_key_evnet: %d,%d\n", event_type, key_value);
    }
}

/*************************************************************************************************/
/*!
 *  \brief      app 事件处理
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
static int multi_event_handler(struct application *app, struct sys_event *event)
{
    switch (event->type) {
    case SYS_KEY_EVENT:
        multi_key_event_handler(event);
        return 0;

    case SYS_BT_EVENT:
        if ((uint32_t)event->arg == SYS_BT_EVENT_TYPE_CON_STATUS) {
            multi_bt_connction_status_event_handler((struct bt_event *)&event->u.bt);
        } else if ((uint32_t)event->arg == SYS_BT_EVENT_TYPE_HCI_STATUS) {
            multi_bt_hci_event_handler((struct bt_event *)&event->u.bt);
        }
        return 0;

    case SYS_DEVICE_EVENT:
        if ((uint32_t)event->arg == DEVICE_EVENT_FROM_POWER) {
            /* return app_power_event_handler(&event->u.dev, multi_set_soft_poweroff); */
        }

    default:
        return FALSE;
    }
    return FALSE;
}

static const struct application_operation app_multi_ops = {
    .state_machine  = multi_state_machine,
    .event_handler 	= multi_event_handler,
};

/*
 * 注册AT Module模式
 */
REGISTER_APPLICATION(app_multi) = {
    .name 	= "multi_conn",
    .action	= ACTION_MULTI_MAIN,
    .ops 	= &app_multi_ops,
    .state  = APP_STA_DESTROY,
};

//-----------------------
//system check go sleep is ok
static uint8_t multi_state_idle_query(void)
{
    return !multi_is_active;
}

REGISTER_LP_TARGET(multi_state_lp_target) = {
    .name = "multi_state_deal",
    .is_idle = multi_state_idle_query,
};

#endif


