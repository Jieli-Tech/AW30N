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

    scan_rx_data_handle(report_pt->data, report_pt->length);
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
    ble_rf_vendor_fixed_channel(36, 3);
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

