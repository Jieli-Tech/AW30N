/* #include "system/app_core.h" */
#include "bt_ble.h"
#include "includes.h"
/* #include "system/includes.h" */
/* #include "server/server_core.h" */
#include "app_action.h"
#include "spple_app_config.h"
/* #include "os/os_api.h" */
#include "btcontroller_config.h"
#include "bt_controller_include/btctrler_task.h"
/* #include "config/config_transport.h" */
#include "bt_include/avctp_user.h"
#include "bt_include/btstack_task.h"
/* #include "bt_common.h" */
/* #include "rcsp_bluetooth.h" */
/* #include "rcsp_user_update.h" */
/* #include "app_charge.h" */
/* #include "app_chargestore.h" */
/* #include "app_power_manage.h" */
#include "app_comm_bt.h"
#include "sys_timer.h"

#define LOG_TAG_CONST       NCON_24G
#define LOG_TAG             "[NCON_24G]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE
#include "log.h"
/* #include "debug.h" */

#if (TRANS_DATA_SPPLE_EN && CONFIG_APP_NONCONN_24G)

#if !(TCFG_USER_BLE_ENABLE && (!TCFG_USER_EDR_ENABLE))
#error "board config error, confirm!!!!!!"
#endif

static u8 is_app_noconn_active = 0;
extern void vPortSuppressTicksAndSleep(u32 usec);
//---------------------------------------------------------------------
void noconn_power_event_to_user(u8 event)
{
    //TODO
    /* struct sys_event e; */
    /* e.type = SYS_DEVICE_EVENT; */
    /* e.arg  = (void *)DEVICE_EVENT_FROM_POWER; */
    /* e.u.dev.event = event; */
    /* e.u.dev.value = 0; */
    /* sys_event_notify(&e); */
}

static void noconn_set_soft_poweroff(void)
{
    log_info("set_soft_poweroff\n");
    is_app_noconn_active = 1;
    //必须先主动断开蓝牙链路,否则要等链路超时断开

#if TCFG_USER_BLE_ENABLE
    btstack_ble_exit(0);
#endif

    power_set_soft_poweroff();
}

static void noconn_app_start()
{
    static u32 i = 0;
    bool temp = 0;

    log_info("=======================================");
    log_info("-----------nonconn_24g demo------------");
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
    extern void cfg_file_parse(u8 idx);
    cfg_file_parse(0);

    btstack_init();
#endif
    /* 按键消息使能 */
    /* sys_key_event_enable(); */
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

static int noconn_state_machine(struct application *app, enum app_state state, struct intent *it)
{
    switch (state) {
    case APP_STA_CREATE:
        break;
    case APP_STA_START:
        if (!it) {
            break;
        }
        switch (it->action) {
        case ACTION_NOCONN_24G_MAIN:
            noconn_app_start();
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

static int noconn_bt_hci_event_handler(struct bt_event *bt)
{
    //对应原来的蓝牙连接上断开处理函数  ,bt->value=reason
    log_info("----%s reason %x %x", __FUNCTION__, bt->event, bt->value);

#if TCFG_USER_BLE_ENABLE
    bt_comm_ble_hci_event_handler(bt);
#endif
    return 0;
}

static int noconn_bt_connction_status_event_handler(struct bt_event *bt)
{
    log_info("----%s %d", __FUNCTION__, bt->event);

#if TCFG_USER_BLE_ENABLE
    bt_comm_ble_status_event_handler(bt);
#endif
    return 0;
}

static void noconn_key_event_handler(struct sys_event *event)
{
    /* u16 cpi = 0; */
    u8 event_type = 0;
    u8 key_value = 0;

    if (event->arg == (void *)DEVICE_EVENT_FROM_KEY) {
        event_type = event->u.key.event;
        key_value = event->u.key.value;
        log_info("app_key_evnet: %d,%d\n", event_type, key_value);

        /* if (event_type == KEY_EVENT_TRIPLE_CLICK */
        /*     && (key_value == TCFG_ADKEY_VALUE3 || key_value == TCFG_ADKEY_VALUE0)) { */
        /*     noconn_power_event_to_user(POWER_EVENT_POWER_SOFTOFF); */
        /*     return; */
        /* } */

    }
}

static int noconn_event_handler(struct application *app, struct sys_event *event)
{
    switch (event->type) {
    case SYS_KEY_EVENT:
        noconn_key_event_handler(event);
        return 0;

    case SYS_BT_EVENT:
#if (TCFG_USER_BLE_ENABLE)
        if ((u32)event->arg == SYS_BT_EVENT_TYPE_CON_STATUS) {
            noconn_bt_connction_status_event_handler(&event->u.bt);
        } else if ((u32)event->arg == SYS_BT_EVENT_TYPE_HCI_STATUS) {
            noconn_bt_hci_event_handler(&event->u.bt);
        }
#endif
        return 0;

    case SYS_DEVICE_EVENT:
        if ((u32)event->arg == DEVICE_EVENT_FROM_POWER) {
            //TODO
            /* return app_power_event_handler(&event->u.dev, noconn_set_soft_poweroff); */
        }

#if TCFG_CHARGE_ENABLE
        else if ((u32)event->arg == DEVICE_EVENT_FROM_CHARGE) {
            app_charge_event_handler(&event->u.dev);
        }
#endif
        return 0;

    default:
        return FALSE;
    }
    return FALSE;
}

static const struct application_operation app_noconn_ops = {
    .state_machine  = noconn_state_machine,
    .event_handler 	= noconn_event_handler,
};

/*
 * 注册AT Module模式
 */
REGISTER_APPLICATION(app_noconn) = {
    .name 	= "nonconn_24g",
    .action	= ACTION_NOCONN_24G_MAIN,
    .ops 	= &app_noconn_ops,
    .state  = APP_STA_DESTROY,
};

//-----------------------
//system check go sleep is ok
static u8 noconn_state_idle_query(void)
{
    return !is_app_noconn_active;
}

REGISTER_LP_TARGET(noconn_state_lp_target) = {
    .name = "noconn_state_deal",
    .is_idle = noconn_state_idle_query,
};

#endif



