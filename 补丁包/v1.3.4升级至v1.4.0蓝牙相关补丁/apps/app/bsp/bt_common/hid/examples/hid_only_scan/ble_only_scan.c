/*********************************************************************************************
    *   Filename        : ble_only_scan.c

    *   Description     :

    *   Author          :

    *   Email           :

    *   Last modifiled  : 2024

    *   Copyright:(c)JIELI  2023-2031  @ , All Rights Reserved.
*********************************************************************************************/
#include "msg.h"
#include "includes.h"
#include "app_config.h"
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
#include "ble_only_scan.h"
#include "user_cfg.h"

// todo change func name

#if (TRANS_DATA_HID_EN)
#if CONFIG_SLAVE_ADD_LE_SCAN
#define LOG_TAG         "[ble_only_scan]"
#include "log.h"

// scan rx
static uint8_t scan_rx_buffer[ADV_RSP_PACKET_MAX * 2];
static uint8_t scan_rx_len = 0;

/*************************************************************************************************/
/*!
 *  \brief      扫描接收到的数据
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
void scan_rx_data_handle(const uint8_t *data, uint16_t len)
{
    putchar('V');
    /* log_info("rx_data: %d", len); */
    /* put_buf((uint8_t *)data, len); */

    //TODO
}

static void le_advertising_report_event_upload(u8 event_type, u8 addr_type, u8 *addr, int len, u8 *data, s8 rssi)
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
            if (!strcmp((const char *)data, "TEST2")) {
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


/*************************************************************************************************/
/*!
 *  \brief      接收机数据处理回调
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note       广播数据过滤处理
 */
/*************************************************************************************************/
void ble_rx_report_handle(adv_report_t *report_pt, uint16_t len)
{
    /* log_info("event_type,addr_type:%x,%x\n", report_pt->event_type, report_pt->address_type); */
    /* log_info_hexdump(report_pt->address, 6); */
    /* static uint32_t scan_packet_num = 0;//for test */
    /* r_printf("rx data:%d",report_pt->length); */
    /* put_buf(report_pt->data,report_pt->length);	 */
    if (report_pt == NULL) {
        return;
    }

    if (0 == report_pt->length) {
        return;
    }

    /* uint8_t data_len = report_pt->length - 1; */
    /*  */
    /* if (!data_len) { */
    /*     return; */
    /* } */

    /* if (report_pt->data[0] == RSP_TX_HEAD) { */
    /*     memcpy(&scan_rx_buffer[ADV_RSP_PACKET_MAX - 1], &report_pt->data[1], data_len); */
    /*     log_info("long_packet =%d\n", scan_rx_len); */
    /* } else { */
    /*     memcpy(scan_rx_buffer, &report_pt->data[1], data_len); */
    /*     scan_rx_len = report_pt->data[0]; */
    /*     if (scan_rx_len > ADV_RSP_PACKET_MAX - 1) { */
    /*         log_info("first_packet =%d,wait next packet\n", data_len); */
    /*         return; */
    /*     } else { */
    /*         log_info("short_packet =%d\n", scan_rx_len); */
    /*     } */
    /* } */

    //for debug
    /* log_info("rssi:%d,packet_num:%u\n", report_pt->rssi, ++scan_packet_num); */
    /* scan_rx_data_handle(scan_rx_buffer, scan_rx_len); */

    //scan_rx_data_handle(report_pt->data, report_pt->length);
    le_advertising_report_event_upload(report_pt->event_type, report_pt->address_type, report_pt->address, report_pt->length, report_pt->data, report_pt->rssi);
    /* scan_rx_len = 0; */
}


/*************************************************************************************************/
/*!
 *  \brief      接收机初始化
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
static void scan_rx_setup_init(void)
{
    ble_op_set_scan_param(SET_SCAN_TYPE, SET_SCAN_INTERVAL, SET_SCAN_WINDOW);
}

/*************************************************************************************************/
/*!
 *  \brief      scan使能
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
void scan_rx_enable(uint8_t enable)
{
    log_info("rx_en:%d\n", enable);
    if (enable) {
        scan_rx_setup_init();
    }
    ble_op_scan_enable2(enable, 0);
}

/*************************************************************************************************/
/*!
 *  \brief     bt init
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
void only_scan_ble_init(void)
{
    log_info("*****scan ble_init******\n");

#if CFG_RF_USE_24G_CDOE
    rf_set_24g_hackable_coded(CFG_RF_24G_CODE_ID);
#endif

#if CFG_RF_ADV_SCAN_CHL
    //通信信道，发送接收都要一致
    ble_rf_vendor_fixed_channel(CFG_RF_ADV_SCAN_CHL, 3);
#endif

    scan_rx_enable(1);
}

/*************************************************************************************************/
/*!
 *  \brief      修改2.4G CODED码
 *  \param      [in] coded         设置coded码为32bits.
 *
 *  \note       在初始化完成后任意非连接时刻修改CODED码
 */
/*************************************************************************************************/
void rf_set_conn_24g_coded(uint32_t coded)
{
    scan_rx_enable(0);
    rf_set_scan_24g_hackable_coded(coded);
    scan_rx_enable(1);
}
/*************************************************************************************************/
/*!
 *  \brief     bt exit
 *
 *  \param      [in]
 *
 *  \return
 *
 *  \note
 */
/*************************************************************************************************/
void only_scan_ble_exit(void)
{
    log_info("***** ble_exit******\n");
    scan_rx_enable(0);
}
#endif
#endif

