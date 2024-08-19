/*********************************************************************************************
    *   Filename        : le_server_module.c

    *   Description     :

    *   Author          :

    *   Email           : zh-jieli.com

    *   Last modifiled  : 2017-01-17 11:14

    *   Copyright:(c)JIELI  2011-2016  @ , All Rights Reserved.
*********************************************************************************************/

// *****************************************************************************
/* EXAMPLE_START(le_counter): LE Peripheral - Heartbeat Counter over GATT
 *
 * @text All newer operating systems provide GATT Client functionality.
 * The LE Counter examples demonstrates how to specify a minimal GATT Database
 * with a custom GATT Service and a custom Characteristic that sends periodic
 * notifications.
 */
// *****************************************************************************
/* #include "system/app_core.h" */
/* #include "system/includes.h" */
#include "bt_ble.h"
#include "includes.h"

#include "spple_app_config.h"
#include "app_action.h"

#include "btcontroller_config.h"
#include "bt_controller_include/btctrler_task.h"
#include "bt_include/btstack_task.h"
#include "bt_include/bluetooth.h"
#include "user_cfg.h"
#include "vm.h"
#include "btcontroller_modules.h"
/* #include "bt_common.h" */
/* #include "3th_profile_api.h" */
/* #include "le_common.h" */

/* #include "rcsp_bluetooth.h" */
/* #include "JL_rcsp_api.h" */
/* #include "custom_cfg.h" */
#include "le_gatt_common.h"
#include "app_comm_bt.h"
#include "sys_timer.h"
#include "msg.h"
#include "ble_reduce_ram.h"

#if (TRANS_DATA_SPPLE_EN && CONFIG_APP_REDUCE_RAM)

#if 1//CONFIG_DEBUG_ENABLE
/* extern void printf_buf(u8 *buf, u32 len); */
#define log_info(x, ...)  printf("[BLE_24G]" x " ", ## __VA_ARGS__)
#define log_info_hexdump  printf_buf
#else
#define log_info(...)
#define log_info_hexdump(...)
#endif

static u8 adv_data_len = 0;
static u8 adv_data[ADV_RSP_PACKET_MAX];//max is 31
static u8 scan_rsp_data_len = 0;
static u8 scan_rsp_data[ADV_RSP_PACKET_MAX];//max is 31

static char gap_device_name[BT_NAME_LEN_MAX] = "jl_ble_test";
static u8 gap_device_name_len = 0;     //名字长度，不包含结束符
static u8 adv_ctrl_en = 0;             //adv控制
static u8 scan_ctrl_en = 0;            //scan控制

//------------------------------------------------------
//广播参数设置
extern const char *bt_get_local_name();
extern void clr_wdt(void);
//------------------------------------------------------
static void tx_setup_init(void)
{
    uint8_t adv_type;
    uint8_t adv_channel = ADV_CHANNEL_ALL;
    int ret = 0;

    if (scan_rsp_data_len) {
        adv_type = ADV_SCAN_IND;
    } else {
        adv_type = ADV_NONCONN_IND;
    }

    ble_op_set_adv_param(ADV_INTERVAL_VAL, adv_type, adv_channel);
    ble_op_set_adv_data(adv_data_len, adv_data);
    /* log_info_hexdump(adv_data,adv_data_len); */

    ble_op_set_rsp_data(scan_rsp_data_len, scan_rsp_data);
    /* log_info_hexdump(scan_rsp_data,scan_rsp_data_len); */
}

//发送使能
static void ble_tx_enable(u8 enable)
{
#if CONFIG_TX_MODE_ENABLE
    if (adv_ctrl_en != enable) {
        adv_ctrl_en = enable;
        log_info("tx_en:%d\n", enable);
        log_info("tx_en:%d\n", enable);
        if (enable) {
            tx_setup_init();
        }
        ble_op_adv_enable(enable);
    }
#endif
}

//发送数据, len support max is 60
int ble_tx_send_data(const u8 *data, u8 len)
{
    if (0 == len || len > (ADV_RSP_PACKET_MAX - 1) * 2) {
        log_info("len is overflow:%d\n", len);
        return -1;
    }

    u8 tmp_len;
    adv_data_len = 0;
    scan_rsp_data_len = 0;

    if (len > ADV_RSP_PACKET_MAX - 1) {
        adv_data_len = ADV_RSP_PACKET_MAX - 1;
    } else {
        adv_data_len = len;
    }

    adv_data[0] = len;//packet len
    memcpy(&adv_data[1], data, adv_data_len);
    data += adv_data_len;
    len -= adv_data_len;

    adv_data_len++; //add head

    if (len) {
        scan_rsp_data[0] = RSP_TX_HEAD;
        scan_rsp_data_len = len + 1; //add head
        memcpy(&scan_rsp_data[1], data, len);
    }

    ble_tx_enable(1);
    //延时确定发送成功
    mdelay(TX_DATA_COUNT * TX_DATA_INTERVAL);
    /* os_time_dly(TX_DATA_COUNT * TX_DATA_INTERVAL / 10 + 1); */
    ble_tx_enable(0);
    return 0;
}

static u8 test_buffer[64];
static void ble_tx_timer_test(void)
{
    static u8 tag_value = 0;
    u8 test_len = 60;
    if (tag_value == 0) {
        for (int i = 0; i < 64; i++) {
            test_buffer[i] = i;
        }
        tag_value++;
    }

    ble_tx_send_data(test_buffer, test_len);
    r_printf("tx_data: %02x", test_buffer[0]);
    log_info_hexdump(&test_buffer, test_len);
    test_buffer[0]++;
}

static u8 rx_buffer[ADV_RSP_PACKET_MAX * 2];
static u8 rx_len = 0;
// *****************************************************************************
//扫描接收到的数据
static void ble_rx_data_handle(const u8 *data, u16 len)
{
    log_info("rx_data: %d", len);
    log_info_hexdump(data, len);
}

static void ble_rx_report_handle(adv_report_t *report_pt, u16 len)
{
    /* log_info("event_type,addr_type:%x,%x\n", report_pt->event_type, report_pt->address_type); */
    /* log_info_hexdump(report_pt->address, 6); */
    static u32 scan_packet_num = 0;//for test

    r_printf("rx data:%d", report_pt->length);
    log_info_hexdump(report_pt->data, report_pt->length);

    return ;

    if (report_pt == NULL) {
        return;
    }

    if (0 == report_pt->length) {
        return;
    }

    u8 data_len = report_pt->length - 1;

    if (!data_len) {
        return;
    }

    if (report_pt->data[0] == RSP_TX_HEAD) {
        memcpy(&rx_buffer[ADV_RSP_PACKET_MAX - 1], &report_pt->data[1], data_len);
        log_info("long_packet =%d\n", rx_len);
    } else {
        memcpy(rx_buffer, &report_pt->data[1], data_len);
        rx_len = report_pt->data[0];
        if (rx_len > ADV_RSP_PACKET_MAX - 1) {
            log_info("first_packet =%d,wait next packet\n", data_len);
            return;
        } else {
            log_info("short_packet =%d\n", rx_len);
        }
    }

    //for debug
    log_info("rssi:%d,packet_num:%u\n", report_pt->rssi, ++scan_packet_num);
    ble_rx_data_handle(rx_buffer, rx_len);
    rx_len = 0;
}

static void ble_rx_event_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size)
{
    putchar('M');
    switch (packet_type) {
    case HCI_EVENT_PACKET:
        switch (hci_event_packet_get_type(packet)) {
        case GAP_EVENT_ADVERTISING_REPORT:
            ble_rx_report_handle((void *)&packet[2], packet[1]);
            break;
        default:
            break;
        }
        break;
    }
}

//协议栈回调
void le_advertising_report_event_upload(u8 subevent_code, u8 event_type, u8 addr_type, u8 *addr, int len, u8 *data, u8 rssi)
{
    u8 i, length, ad_type;
    u32 tmp32;

    /* r_printf("rx data:%d, rssi:%d \n",len, rssi); */
    /* log_info_hexdump(data, len); */
    /* log_info_hexdump(addr, 6); */

    for (i = 0; i < len;) {

        if (*data == 0) {
            /* log_info("analyze end\n"); */
            break;
        }

        length = *data++;

        if (length >= len || (length + i) >= len) {
            /*过滤非标准包格式*/
            putchar('^');
            /* printf("!!!error_adv_packet:"); */
            /* log_info_hexdump(data, len); */
            break;
        }

        ad_type = *data++;
        i += (length + 1);

        switch (ad_type) {
        case HCI_EIR_DATATYPE_FLAGS:
            /* log_info("flags:%02x\n",adv_data_pt[0]); */
            break;

        case HCI_EIR_DATATYPE_MORE_16BIT_SERVICE_UUIDS:
        case HCI_EIR_DATATYPE_COMPLETE_16BIT_SERVICE_UUIDS:
        case HCI_EIR_DATATYPE_MORE_32BIT_SERVICE_UUIDS:
        case HCI_EIR_DATATYPE_COMPLETE_32BIT_SERVICE_UUIDS:
        case HCI_EIR_DATATYPE_MORE_128BIT_SERVICE_UUIDS:
        case HCI_EIR_DATATYPE_COMPLETE_128BIT_SERVICE_UUIDS:
            /* log_info("service uuid:"); */
            /* log_info_hexdump(adv_data_pt, length - 1); */
            break;

        case HCI_EIR_DATATYPE_COMPLETE_LOCAL_NAME:
        case HCI_EIR_DATATYPE_SHORTENED_LOCAL_NAME:
            tmp32 = data[length - 1];
            data[length - 1] = 0;;
            log_info("remoter_name:%s ,rssi:%d\n", data,  rssi);
            if (!strcmp(data, "TEST")) {
                post_msg(1, MSG_TEST_DEMO);
            }
            log_info_hexdump(addr, 6);
            data[length - 1] = tmp32;
            break;

        case HCI_EIR_DATATYPE_MANUFACTURER_SPECIFIC_DATA:
            break;

        case HCI_EIR_DATATYPE_APPEARANCE_DATA:
            /* log_info("get_class_type:%04x\n",little_endian_read_16(adv_data_pt,0)); */
            break;

        default:
            /* log_info("unknow ad_type:"); */
            break;
        }
    }


}

static void rx_setup_init(void)
{
    /* ble_op_set_scan_param(SET_SCAN_TYPE, SET_SCAN_INTERVAL, SET_SCAN_WINDOW); */
    ll_hci_scan_set_params(SET_SCAN_TYPE, SET_SCAN_INTERVAL, SET_SCAN_WINDOW);
}


struct scan_enable_parameter {
    u8 enable;
    u8 filter_duplicates;
} _GNU_PACKED_;
extern int ll_scan_open(const u8 *data);

//收数使能
static void ble_rx_enable(u8 enable)
{
    struct scan_enable_parameter scan_enable;
#if CONFIG_RX_MODE_ENABLE
    if (scan_ctrl_en != enable) {
        scan_ctrl_en = enable;
        log_info("rx_en:%d\n", enable);
        if (enable) {
            rx_setup_init();
        }
        scan_enable.enable = enable;
        scan_enable.filter_duplicates = 0;
        ll_scan_open(&scan_enable);
        /* ble_op_scan_enable(enable); */
    }
#endif
}

static const u8 test_tx_data[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
static const char ble_ext_name[] = "(BLE)";

//------------------------------------------------------------
//协议栈内部调用
void ble_profile_init(void)
{
    log_info("ble profile init\n");
    // register for HCI events
    hci_event_callback_set(&ble_rx_event_handler);
    /* le_l2cap_register_packet_handler(&ble_rx_packet_handler); */
}

void ble_module_enable(u8 en)
{
    log_info("mode_en:%d\n", en);
    if (en) {
    } else {
        ble_rx_enable(0);
        ble_tx_enable(0);
    }
}

void bt_ble_init(void)
{
    log_info("***** ble_init******\n");
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

#if CONFIG_TX_MODE_ENABLE
    //for test
    sys_timer_add(NULL, ble_tx_timer_test, 3000);
#endif

#if CONFIG_RX_MODE_ENABLE
    ble_rx_enable(1);
#endif

}

/*************************************************************************************************/
/*!
 *  \brief      修改2.4G CODED码
 *  \param      [in] coded         设置coded码为<=24bits.
 *  \param      [in] channel       设置广播(GATT_ROLE_SERVER) or 扫描(GATT_ROLE_CLIENT)
 *
 *  \note       在初始化完成后任意非连接时刻修改CODED码
 */
/*************************************************************************************************/
void rf_set_conn_24g_coded(u32 coded, u8 channel)
{
    if (channel == GATT_ROLE_CLIENT) {
        ble_rx_enable(0);
        rf_set_scan_24g_hackable_coded(coded);
        ble_rx_enable(1);
    } else {
        ble_tx_enable(0);
        rf_set_adv_24g_hackable_coded(coded);
        ble_tx_enable(1);
    }
}

void bt_ble_exit(void)
{
    log_info("***** ble_exit******\n");
    ble_module_enable(0);
}

void ble_server_send_test_key_num(u8 key_num)
{
    ;
}

void ble_app_disconnect(void)
{
}

void bt_ble_adv_enable(u8 enable)
{

}

#endif


