
/* #include "system/app_core.h" */
#include "bt_ble.h"
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
/* #include "rcsp_bluetooth.h" //TODO*/
/* #include "rcsp_user_update.h"//TODO */
/* #include "app_charge.h" */
/* #include "app_power_manage.h" */
/* #include "le_client_demo.h" */
#include "usb/device/hid.h"
#include "usb/device/cdc.h"
#include "vble_complete.h"
#include "app_comm_bt.h"
#include "sys_timer.h"

#define LOG_TAG_CONST       DONGLE
#define LOG_TAG             "[DONGLE]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE
/* #include "debug.h" */
#include "log.h"

#if (TRANS_DATA_SPPLE_EN && CONFIG_APP_DONGLE)

//2.4G模式: 0---ble, 非0---2.4G配对码
#define CFG_RF_24G_CODE_ID       (0) //<=24bits
/* #define CFG_RF_24G_CODE_ID       (0x23) //<=24bits */

//---------------------------------------------------------------------
extern void dongle_custom_hid_rx_handler(void *priv, u8 *buf, u32 len);
extern int ble_hid_data_send_ext(u8 report_type, u8 report_id, u8 *data, u16 len);
#if TCFG_USER_BLE_ENABLE
extern void dongle_ota_init(void);
extern void dongle_return_online_list(void);
#endif
extern void custom_hid_init(void);
extern int dongle_pc_event_handler(struct dg_ota_event *dg_ota);
extern int dongle_otg_event_handler(struct dg_ota_event *dg_ota);
extern void custom_hid_set_rx_hook(void *priv, void (*rx_hook)(void *priv, u8 *buf, u32 len));
//---------------------------------------------------------------------
extern void bt_pll_para(u32 osc, u32 sys, u8 low_power, u8 xosc);
static void dongle_app_start()
{
    printf("=======================================");
    printf("---------usb + dongle demo---------");
    printf("=======================================");
    printf("app_file: %s", __FILE__);

#if TCFG_USER_BLE_ENABLE
    /* SFR(JL_CLOCK->PRP_CON1,  0, 3, 1); */
    /* SFR(JL_CLOCK->PRP_CON1, 26, 1, 1); */
    SFR(JL_CLOCK->PRP_CON2, 0, 2, 1);   // wl_and_clk 1:std_48m
    SFR(JL_CLOCK->PRP_CON2, 2, 2, 2);   // wl2adc_clk 1:std_48m 2:pll_96m
    SFR(JL_CLOCK->PRP_CON2, 4, 2, 2);   // wl2dac_clk 1:std_48m 2:pll_96m
    SFR(JL_CLOCK->PRP_CON2, 8, 1, 1);   // wl_aud_rst
    asm("csync");

    bt_pll_para(48000000, 48000000, 0, 0);

    btstack_ble_start_before_init(NULL, 0);
    rf_set_24g_hackable_coded(CFG_RF_24G_CODE_ID);
    extern void cfg_file_parse(u8 idx);
    cfg_file_parse(0);

    btstack_init();

    //btstack task 使用软中断执行
    /* extern void btstack_task(void); */
    /* extern const int IRQ_BTSTACK_MSG_IP; */
    /* request_irq(IRQ_SOFT3_IDX, IRQ_BTSTACK_MSG_IP, btstack_task, 0); */
#endif

//ota升级初始化——预留
#if RCSP_BTMATE_EN
    custom_hid_set_rx_hook(NULL, dongle_custom_hid_rx_handler);//重注册接收回调到dongle端
    /* download_buf = malloc(1024); */
    /* dongle_return_online_list(); */
#if TCFG_USER_BLE_ENABLE
    dongle_ota_init();
    sys_timeout_add(NULL, dongle_return_online_list, 4000);
#endif
#endif
}

void ble_master_init(void)
{
    btstack_ble_start_before_init(NULL, 0);
    rf_set_24g_hackable_coded(CFG_RF_24G_CODE_ID);
    extern void cfg_file_parse(u8 idx);
    cfg_file_parse(0);

    btstack_init();
#if RCSP_BTMATE_EN
    custom_hid_set_rx_hook(NULL, dongle_custom_hid_rx_handler);//重注册接收回调到dongle端
    /* download_buf = malloc(1024); */
    /* dongle_return_online_list(); */
#if TCFG_USER_BLE_ENABLE
    dongle_ota_init();
    sys_timeout_add(NULL, dongle_return_online_list, 4000);
#endif
#endif
}

void bt_init_api(void)
{
    dongle_app_start();
}

//ble 接收设备数据
int dongle_ble_hid_input_handler(u8 *packet, u16 size)
{
    /* log_info("ble_hid_data_input:size=%d", size); */
    put_buf(packet, size);

    /* rf_unpack_data(packet, size); */
#if (TCFG_PC_ENABLE && TCFG_BT_PC_ENABLE)
    putchar('&');
    return hid_send_data(packet, size);
#else
    log_info("chl1 disable!!!\n");
    return 0;
#endif
}

//ble 接收第二个设备数据
int dongle_second_ble_hid_input_handler(u8 *packet, u16 size)
{
    /* log_info("ble_hid_data_input:size=%d", size); */
    /* put_buf(packet, size); */

    putchar('#');
#if (TCFG_PC_ENABLE && TCFG_BT_PC_ENABLE) && (CONFIG_BT_GATT_CLIENT_NUM == 2)
    return hid_send_second_data(packet, size);
#else
    log_info("chl2 disable!!!\n");
    return 0;
#endif
}

//usb send api   预留
void dongle_hid_output_handler(u8 report_type, u8 report_id, u8 *packet, u16 size)
{
    log_info("hid_data_output:type= %02x,report_id= %d,size=%d", report_type, report_id, size);
    put_buf(packet, size);

#if TCFG_USER_BLE_ENABLE
    ble_hid_data_send_ext(report_type, report_id, packet, size);
#endif
}

//dongle状态机
static int dongle_state_machine(struct application *app, enum app_state state, struct intent *it)
{
    switch (state) {
    case APP_STA_CREATE:
        break;
    case APP_STA_START:
        if (!it) {
            break;
        }
        switch (it->action) {
        case ACTION_DONGLE_MAIN:
            dongle_app_start();
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

static void dongle_bt_ble_status_callback(ble_state_e status, u8 reason)
{
    log_info("dongle_bt_ble_status_callback============== %02x   reason:0x%x\n", status, reason);

    vble_master_status_update(status, reason);
    switch (status) {
    case BLE_ST_CONNECT:
        log_info("BLE_ST_CONNECT\n");
        /* post_msg(1, MSG_BLE_CONNECT_COMPLETE); */
        break;

    case BLE_PRIV_MSG_PAIR_CONFIRM:     //发起加密，等待上层用户确认,暂未添加
        log_info("BLE_PRIV_MSG_PAIR_CONFIRM\n");
        break;

    case BLE_PRIV_PAIR_ENCRYPTION_CHANGE:
        log_info("BLE_PRIV_PAIR_ENCRYPTION_CHANGE\n");
        /* post_msg(1, MSG_BLE_CONNECT_COMPLETE); */
        break;

    case BLE_ST_SEND_DISCONN:
        log_info("BLE_ST_SEND_DISCONN\n");
        /* post_msg(1, MSG_BLE_DISCONNECT_COMPLETE); */
        break;

    case BLE_ST_DISCONN:
        log_info("BLE_ST_DISCONN BY LOCAL\n");
        /* post_msg(1, MSG_BLE_DISCONNECT_COMPLETE); */
        break;

    case BLE_ST_SEARCH_COMPLETE:     //与从机的notify indicatei作用相同
        log_info("BLE_ST_SEARCH_COMPLETE\n");
        /* post_msg(1, MSG_BLE_CONNECT_COMPLETE); */
        break;

    default:
        break;
    }
}

static int dongle_bt_hci_event_handler(struct bt_event *bt)
{
    //对应原来的蓝牙连接上断开处理函数  ,bt->value=reason
    log_info("----%s reason %x %x", __FUNCTION__, bt->event, bt->value);

#if TCFG_USER_BLE_ENABLE
    bt_comm_ble_hci_event_handler(bt);
#endif
    return 0;
}

static int dongle_bt_connction_status_event_handler(struct bt_event *bt)
{
    log_info("----%s %d", __FUNCTION__, bt->event);

#if TCFG_USER_BLE_ENABLE
    bt_comm_ble_status_event_handler(bt);
#endif
    return 0;
}

static int dongle_event_handler(struct application *app, struct sys_event *event)
{
    switch (event->type) {

    case SYS_BT_EVENT:
#if TCFG_USER_BLE_ENABLE
        if ((u32)event->arg == SYS_BT_EVENT_TYPE_CON_STATUS) {
            dongle_bt_connction_status_event_handler(&event->u.bt);
        } else if ((u32)event->arg == SYS_BT_EVENT_BLE_STATUS) {
            dongle_bt_ble_status_callback(event->u.bt.event, event->u.bt.value);
        } else if ((u32)event->arg == SYS_BT_EVENT_TYPE_HCI_STATUS) {
            dongle_bt_hci_event_handler(&event->u.bt);
#if RCSP_BTMATE_EN
        } else if ((u32)event->arg == DEVICE_EVENT_FROM_PC) {
            dongle_pc_event_handler(&event->u.bt);//dongle pc命令回调处理
        } else if ((u32)event->arg == DEVICE_EVENT_FROM_OTG) {
            dongle_otg_event_handler(&event->u.bt);//dongle ota升级数据透传
#endif
        }
#endif
        return 0;

    default:
        return FALSE;
    }
    return FALSE;
}

/////////////////////////////////////////////////////////////////////////
static const struct application_operation app_dongle_ops = {
    .state_machine  = dongle_state_machine,
    .event_handler 	= dongle_event_handler,
};

/*
 * 注册AT Module模式
 */
REGISTER_APPLICATION(app_dongle) = {
    .name 	= "dongle",
    .action	= ACTION_DONGLE_MAIN,
    .ops 	= &app_dongle_ops,
    .state  = APP_STA_DESTROY,
};

#endif


