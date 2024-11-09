#ifndef _BLE_ONLY_SCAN_H
#define _BLE_ONLY_SCAN_H
#include "typedef.h"

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
#define SET_SCAN_INTERVAL   ADV_SCAN_MS(100)//unit: 0.625ms
//搜索 窗口大小
#define SET_SCAN_WINDOW     ADV_SCAN_MS(50)//unit: 0.625ms

#endif
