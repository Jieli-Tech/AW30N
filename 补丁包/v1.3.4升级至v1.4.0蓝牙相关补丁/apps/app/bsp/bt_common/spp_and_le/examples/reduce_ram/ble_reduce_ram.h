#ifndef _BLE_REDUCE_RAM_H
#define _BLE_REDUCE_RAM_H
#include "typedef.h"

//配置收发角色
#define CONFIG_TX_MODE_ENABLE     0 //发射器
#define CONFIG_RX_MODE_ENABLE     1 //接收器

//------------------------------------------------------
#define CFG_RF_USE_24G_CDOE       0  // 是否使用24识别码

#if CFG_RF_USE_24G_CDOE
#define CFG_RF_ADV_SCAN_CHL       36//0-(默认37，38，39), 其他配置值 1~39
#define CFG_RF_24G_CODE_ID        0x5DA086D2 // 24g 识别码(32bit),发送接收都要匹配:!!!初始化之后任意非连接时刻修改配对码API:rf_set_conn_24g_coded
#else
#define CFG_RF_ADV_SCAN_CHL       0//0-(默认37，38，39), 其他配置值 1~39
#define CFG_RF_24G_CODE_ID        0//
#endif

//------------------------------------------------------
//RX接收配置
#define ADV_SCAN_MS(_ms)    ((_ms) * 8 / 5)
//搜索类型
#define SET_SCAN_TYPE       SCAN_ACTIVE
//搜索 周期大小
#define SET_SCAN_INTERVAL   ADV_SCAN_MS(24)//unit: 0.625ms
//搜索 窗口大小
#define SET_SCAN_WINDOW     ADV_SCAN_MS(8)//unit: 0.625ms

//TX发送配置
#define TX_DATA_COUNT             3  //发送次数,决定os_time_dly 多久
#define TX_DATA_INTERVAL          20 //发送间隔>=20ms

#define ADV_INTERVAL_VAL          ADV_SCAN_MS(TX_DATA_INTERVAL)//unit: 0.625ms
#define RSP_TX_HEAD               0xff


#endif
