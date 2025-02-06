
/* #include "system/app_core.h" */
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
/* #include "bt_common.h" */
#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".ble_app_bss")
#pragma data_seg(".ble_app_data")
#pragma const_seg(".ble_app_text_const")
#pragma code_seg(".ble_app_text")
#endif

#if (TRANS_DATA_HID_EN)
#if (CONFIG_APP_KEYBOARD)
#include "edr_hid_user.h"
/* #include "code_switch.h" */
/* #include "omsensor/OMSensor_manage.h" */
/* #include "le_common.h" */
#include <stdlib.h>
#include "standard_hid.h"
/* #include "app_charge.h" */
/* #include "app_power_manage.h" */
/* #include "app_chargestore.h" */
#include "app_comm_bt.h"
#include "vble_complete.h"
#include "sys_timer.h"
#include "msg.h"

#if RCSP_BTMATE_EN
#include "rcsp_bluetooth.h"
#endif

#define LOG_TAG_CONST       HID_KEY
#define LOG_TAG             "[HID_KEY]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE
/* #include "debug.h" */
#include "log.h"

//----------------------------------
static const u8 hidkey_report_map[] = {
    0x05, 0x0C,        // Usage Page (Consumer)
    0x09, 0x01,        // Usage (Consumer Control)
    0xA1, 0x01,        // Collection (Application)
    0x85, 0x01,        //   Report ID (1)
    0x09, 0xE9,        //   Usage (Volume Increment)
    0x09, 0xEA,        //   Usage (Volume Decrement)
    0x09, 0xCD,        //   Usage (Play/Pause)
    0x09, 0xE2,        //   Usage (Mute)
    0x09, 0xB6,        //   Usage (Scan Previous Track)
    0x09, 0xB5,        //   Usage (Scan Next Track)
    0x09, 0xB3,        //   Usage (Fast Forward)
    0x09, 0xB4,        //   Usage (Rewind)
    0x15, 0x00,        //   Logical Minimum (0)
    0x25, 0x01,        //   Logical Maximum (1)
    0x75, 0x01,        //   Report Size (1)
    0x95, 0x10,        //   Report Count (16)
    0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0xC0,              // End Collection
    // 35 bytes
};

// consumer key
#define CONSUMER_VOLUME_INC             0x0001
#define CONSUMER_VOLUME_DEC             0x0002
#define CONSUMER_PLAY_PAUSE             0x0004
#define CONSUMER_MUTE                   0x0008
#define CONSUMER_SCAN_PREV_TRACK        0x0010
#define CONSUMER_SCAN_NEXT_TRACK        0x0020
#define CONSUMER_SCAN_FRAME_FORWARD     0x0040
#define CONSUMER_SCAN_FRAME_BACK        0x0080

//----------------------------------
static const u16 hid_key_click_table[8] = {
    CONSUMER_PLAY_PAUSE,
    CONSUMER_SCAN_PREV_TRACK,
    CONSUMER_VOLUME_DEC,
    CONSUMER_SCAN_NEXT_TRACK,
    CONSUMER_VOLUME_INC,
    CONSUMER_MUTE,
    0,
    0,
};

static const u16 hid_key_hold_table[8] = {
    0,
    CONSUMER_SCAN_FRAME_BACK,
    CONSUMER_VOLUME_DEC,
    CONSUMER_SCAN_FRAME_FORWARD,
    CONSUMER_VOLUME_INC,
    0,
    0,
    0,
};

//----------------------------------
static const ble_init_cfg_t hidkey_ble_config = {
    .same_address = 0,
    .appearance = BLE_APPEARANCE_HID_KEYBOARD,
    .report_map = hidkey_report_map,
    .report_map_size = sizeof(hidkey_report_map),
};

/*************************************************************************************************/
/*!
 *  \brief      app 入口
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
extern void bt_pll_para(u32 osc, u32 sys, u8 low_power, u8 xosc);
extern void le_hogp_set_reconnect_adv_cfg(u8 adv_type, u32 adv_timeout);
extern void le_hogp_set_output_callback(void *cb);
static int hid_keyboard_recv_callback(u8 *buffer, u16 size);
static void hidkey_app_start()
{
    log_info("=======================================");
    log_info("-------------HID DEMO-----------------");
    log_info("=======================================");
    log_info("app_file: %s", __FILE__);

#if TCFG_USER_BLE_ENABLE
    /* SFR(JL_CLOCK->PRP_CON1,  0, 3, 1); */
    /* SFR(JL_CLOCK->PRP_CON1, 26, 1, 1); */
    SFR(JL_CLOCK->PRP_CON2, 0, 2, 1);   // wl_and_clk 1:std_48m
    SFR(JL_CLOCK->PRP_CON2, 2, 2, 2);   // wl2adc_clk 1:std_48m 2:pll_96m
    SFR(JL_CLOCK->PRP_CON2, 4, 2, 2);   // wl2dac_clk 1:std_48m 2:pll_96m
    SFR(JL_CLOCK->PRP_CON2, 8, 1, 1);   // wl_aud_rst
    asm("csync");

    bt_pll_para(48000000, 48000000, 0, 0);

    btstack_ble_start_before_init(&hidkey_ble_config, 0);
    /* le_hogp_set_reconnect_adv_cfg(ADV_IND, 5000); */
    le_hogp_set_reconnect_adv_cfg(ADV_DIRECT_IND_LOW, 5000);
    //注册hogp数据接收回调函数
    le_hogp_set_output_callback(hid_keyboard_recv_callback);

    btstack_init();

    //btstack task 使用软中断执行
    /* extern void btstack_task(void); */
    /* extern const int IRQ_BTSTACK_MSG_IP; */
    /* request_irq(IRQ_SOFT3_IDX, IRQ_BTSTACK_MSG_IP, btstack_task, 0); */
#endif
}

void ble_slave_init(void)
{
    btstack_ble_start_before_init(&hidkey_ble_config, 0);
    /* le_hogp_set_reconnect_adv_cfg(ADV_IND, 5000); */
    le_hogp_set_reconnect_adv_cfg(ADV_DIRECT_IND_LOW, 5000);
    //注册hogp数据接收回调函数
    le_hogp_set_output_callback(hid_keyboard_recv_callback);

    btstack_init();
}

void bt_init_api(void)
{
    hidkey_app_start();
}

/*************************************************************************************************/
/*!
 *  \brief      hid 数据接收
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
static int hid_keyboard_recv_callback(u8 *buffer, u16 size)
{
    log_info("ble_hid_data_receive:size=%d", size);
    put_buf(buffer, size);

    //To do

    return 0;
}

/*************************************************************************************************/
/*!
 *  \brief      app  状态处理
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
static int hidkey_state_machine(struct application *app, enum app_state state, struct intent *it)
{
    switch (state) {
    case APP_STA_CREATE:
        break;
    case APP_STA_START:
        if (!it) {
            break;
        }
        switch (it->action) {
        case ACTION_HID_MAIN:
            hidkey_app_start();
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

/*************************************************************************************************/
/*!
 *  \brief      蓝牙HCI事件消息处理
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
static int hidkey_bt_hci_event_handler(struct bt_event *bt)
{
    //对应原来的蓝牙连接上断开处理函数  ,bt->value=reason
    log_info("----%s reason %x %x", __FUNCTION__, bt->event, bt->value);

#if TCFG_USER_BLE_ENABLE
    bt_comm_ble_hci_event_handler((struct bt_event *)bt);
#endif

    return 0;
}

/*************************************************************************************************/
/*!
 *  \brief      蓝牙连接状态事件消息处理
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
static int hidkey_bt_connction_status_event_handler(struct bt_event *bt)
{
    log_info("----%s %d", __FUNCTION__, bt->event);

    switch (bt->event) {
    case BT_STATUS_INIT_OK:
        /*
         * 蓝牙初始化完成
         */
        log_info("BT_STATUS_INIT_OK\n");

        //根据模式执行对应蓝牙的初始化
#if TCFG_USER_BLE_ENABLE
        btstack_ble_start_after_init(0);
#endif
        break;

    default:

#if TCFG_USER_BLE_ENABLE
        bt_comm_ble_status_event_handler((struct bt_event *)bt);
#endif
        break;
    }
    return 0;
}

/*************************************************************************************************/
/*!
 *  \brief      蓝牙ble hogp状态消息回调
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
static void hidkey_hogp_ble_status_callback(ble_state_e status, u8 reason)
{
    log_info("hidkey_hogp_ble_status_callback============== %02x   reason:0x%x\n", status, reason);

    vble_slave_status_update(status, reason);
    switch (status) {
    case BLE_ST_CONNECT:
        log_info("BLE_ST_CONNECT\n");
        post_msg(1, MSG_BLE_CONNECT_COMPLETE);
        break;

    case BLE_PRIV_MSG_PAIR_CONFIRM:
        log_info("BLE_PRIV_MSG_PAIR_CONFIRM\n");
        break;

    case BLE_PRIV_PAIR_ENCRYPTION_CHANGE:
        log_info("BLE_PRIV_PAIR_ENCRYPTION_CHANGE\n");
        post_msg(1, MSG_BLE_CONNECT_COMPLETE);
        break;

    case BLE_ST_SEND_DISCONN:
        log_info("BLE_ST_SEND_DISCONN\n");
        post_msg(1, MSG_BLE_DISCONNECT_COMPLETE);
        break;

    case BLE_ST_DISCONN:
        log_info("BLE_ST_DISCONN BY LOCAL\n");
        post_msg(1, MSG_BLE_DISCONNECT_COMPLETE);
        break;

    case BLE_ST_NOTIFY_IDICATE:
        log_info("BLE_ST_NOTIFY_IDICATE\n");
        post_msg(1, MSG_BLE_CONNECT_COMPLETE);
        break;

    default:
        break;
    }
}
/*************************************************************************************************/
/*!
 *  \brief      蓝牙公共消息处理
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
static int hidkey_bt_common_event_handler(struct bt_event *bt)
{
    log_info("----%s reason %x %x", __FUNCTION__, bt->event, bt->value);

    switch (bt->event) {
    case COMMON_EVENT_BLE_REMOTE_TYPE:
        log_info(" COMMON_EVENT_BLE_REMOTE_TYPE,%d \n", bt->value);
        break;

    case COMMON_EVENT_SHUTDOWN_DISABLE:
        /* hidkey_auto_shutdown_disable(); */
        break;

    default:
        break;

    }
    return 0;
}

/*************************************************************************************************/
/*!
 *  \brief      app 线程事件处理
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
static int hidkey_event_handler(struct application *app, struct sys_event *event)
{

    switch (event->type) {
    case SYS_BT_EVENT:
#if TCFG_USER_BLE_ENABLE
        if ((u32)event->arg == SYS_BT_EVENT_TYPE_CON_STATUS) {
            hidkey_bt_connction_status_event_handler((struct bt_event *)&event->u.bt);
        } else if ((u32)event->arg == SYS_BT_EVENT_TYPE_HCI_STATUS) {
            hidkey_bt_hci_event_handler((struct bt_event *)&event->u.bt);
        } else if ((u32)event->arg == SYS_BT_EVENT_BLE_STATUS) {
            hidkey_hogp_ble_status_callback(event->u.bt.event, event->u.bt.value);
        } else if ((u32)event->arg == SYS_BT_EVENT_FORM_COMMON) {
            return hidkey_bt_common_event_handler((struct bt_event *)&event->u.dev);
        }
#endif
        return 0;

    default:
        return 0;
    }

    return 0;
}

//////////////////////////////////////////////////////////////////////////
static const struct application_operation app_hidkey_ops = {
    .state_machine  = hidkey_state_machine,
    .event_handler 	= hidkey_event_handler,
};

/*
 * 注册模式
 */
REGISTER_APPLICATION(app_hidkey) = {
    .name 	= "hid_key",
    .action	= ACTION_HID_MAIN,
    .ops 	= &app_hidkey_ops,
    .state  = APP_STA_DESTROY,
};


#endif
#endif

