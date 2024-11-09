#ifndef TEST_APP_CONFIG_H
#define TEST_APP_CONFIG_H

#include "bt_ble.h"

#if (TESTE_BLE_EN)
//app case 选择,只选1,要配置对应的board_config.h

#if !(BLE_WIRELESS_1T1_TX_EN || BLE_WIRELESS_1T1_RX_EN || BLE_WIRELESS_1TN_TX_EN || BLE_WIRELESS_1TN_RX_EN)
#define MASTER                          1
#define SLAVE                           1
#else
#define MASTER                          0
#define SLAVE                           0
#define LOWPOWER_PAIR_MODE				1//配置为0为低功耗配对模式，1为快速配对模式
#define WIRELESS_PAIR_BONDING           0
#define FD_AOA_TEST                     1//aoa测试,添加上层对应的feature和代码
#endif

#define CONFIG_IOT_ENABLE               0//配置蓝牙接外部APP的应用

//bt_test gatt
#define DOUBLE_BT_SAME_NAME             0 //同名字
#define DOUBLE_BT_SAME_MAC              0 //同地址
#define CONFIG_HOGP_COMMON_ENABLE       0 //公共的hogp
#define CONFIG_BT_GATT_COMMON_ENABLE    1 //配置使用gatt公共模块
#define CONFIG_BT_SM_SUPPORT_ENABLE     1 //配置是否支持加密
#if MASTER
#define CONFIG_BT_GATT_CLIENT_NUM       1 //配置主机client个数(app not support)
#else
#define CONFIG_BT_GATT_CLIENT_NUM       0 //配置主机client个数(app not support)
#endif
#if SLAVE
#define CONFIG_BT_GATT_SERVER_NUM       1 //配置从机server个数
#else
#define CONFIG_BT_GATT_SERVER_NUM       0 //配置从机server个数
#endif
#define CONFIG_BT_GATT_CONNECTION_NUM   1 //(test 应用仅支持1条链路主从切换)(CONFIG_BT_GATT_SERVER_NUM + CONFIG_BT_GATT_CLIENT_NUM) //配置连接个数
#define CONFIG_BLE_HIGH_SPEED           1 //BLE提速模式: 使能DLE(2M暂时不开,对讲机距离要求), payload要匹配pdu的包长
#define CONFIG_BLE_CONNECT_SLOT         0 //BLE高回报率设置, 支持私有协议
#define PHY_CODE_TEST                   0 //CODED s2 s8认证
#if (PHY_CODE_TEST)
#undef CONFIG_BLE_HIGH_SPEED
#define CONFIG_BLE_HIGH_SPEED           0
#endif
#define CONFIG_BT_LITTLE_BUFFER_MODE    1 //配置蓝牙小buffer模式,数据响应更快,由应用层做缓存。优点：收发响应快，应用层可做丢数(BT ACL不支持丢数)

#define PADVB_WL_MODE                   1 //广播对讲机模式

#define SNIFF_MODE_RESET_ANCHOR         0

#ifndef BT_NV_RAM_SIZE
#define BT_NV_RAM_SIZE                  (0x2800)  //bt nv malloc堆的大小
#endif

//需要app(BLE)升级要开一下宏定义
#if CONFIG_APP_OTA_EN
#define RCSP_UPDATE_EN                  1
#define UPDATE_MD5_ENABLE               0
#else
#define RCSP_UPDATE_EN                  0
#define UPDATE_MD5_ENABLE               0
#endif


#if (CONFIG_BT_MODE == BT_NORMAL)
//enable dut mode,need disable sleep(TCFG_LOWPOWER_LOWPOWER_SEL = 0)
// #define BLE_DUT_TEST                  0
#if BLE_DUT_TEST
#undef  TCFG_LOWPOWER_LOWPOWER_SEL
#define TCFG_LOWPOWER_LOWPOWER_SEL    0

#undef TCFG_HID_AUTO_SHUTDOWN_TIME
#define TCFG_HID_AUTO_SHUTDOWN_TIME   0

#undef  TCFG_USER_TWS_ENABLE
#define TCFG_USER_TWS_ENABLE          0     //tws功能使能

#undef TCFG_PC_ENABLE
#define TCFG_PC_ENABLE				  0     //PC模块

#undef TCFG_UDISK_ENABLE
#define TCFG_UDISK_ENABLE			  0

#undef POWERDOWN_UDISK_MODE_EN
#define POWERDOWN_UDISK_MODE_EN		  0

#undef APP_SOFTOFF_CNT_TIME_10MS
#define APP_SOFTOFF_CNT_TIME_10MS     0

#endif

#else

#define BLE_DUT_TEST                  0

#undef  TCFG_BD_NUM
#define TCFG_BD_NUM						          1

#undef  TCFG_USER_TWS_ENABLE
#define TCFG_USER_TWS_ENABLE                      0     //tws功能使能

#undef  TCFG_USER_BLE_ENABLE
#define TCFG_USER_BLE_ENABLE                      1     //BLE功能使能

#undef  TCFG_AUTO_SHUT_DOWN_TIME
#define TCFG_AUTO_SHUT_DOWN_TIME		          0

#undef  TCFG_SYS_LVD_EN
#define TCFG_SYS_LVD_EN						      0

#undef  TCFG_LOWPOWER_LOWPOWER_SEL
#define TCFG_LOWPOWER_LOWPOWER_SEL                0

#undef TCFG_AUDIO_DAC_LDO_VOLT
#define TCFG_AUDIO_DAC_LDO_VOLT				DACVDD_LDO_2_65V

#undef TCFG_LOWPOWER_POWER_SEL
#define TCFG_LOWPOWER_POWER_SEL				PWR_LDO15

#undef  TCFG_PWMLED_ENABLE
#define TCFG_PWMLED_ENABLE					DISABLE_THIS_MOUDLE

#undef  TCFG_ADKEY_ENABLE
#define TCFG_ADKEY_ENABLE                   DISABLE_THIS_MOUDLE

#undef  TCFG_IOKEY_ENABLE
#define TCFG_IOKEY_ENABLE					DISABLE_THIS_MOUDLE

#undef TCFG_TEST_BOX_ENABLE
#define TCFG_TEST_BOX_ENABLE			    0

#undef TCFG_AUTO_SHUT_DOWN_TIME
#define TCFG_AUTO_SHUT_DOWN_TIME	        0

#undef TCFG_POWER_ON_NEED_KEY
#define TCFG_POWER_ON_NEED_KEY		        0

// #undef TCFG_UART0_ENABLE
// #define TCFG_UART0_ENABLE					DISABLE_THIS_MOUDLE

#undef TCFG_HID_AUTO_SHUTDOWN_TIME
#define TCFG_HID_AUTO_SHUTDOWN_TIME             0

#undef TCFG_PC_ENABLE
#define TCFG_PC_ENABLE				  0     //PC模块

#undef TCFG_UDISK_ENABLE
#define TCFG_UDISK_ENABLE			  0

#undef POWERDOWN_UDISK_MODE_EN
#define POWERDOWN_UDISK_MODE_EN		  0

#undef APP_SOFTOFF_CNT_TIME_10MS
#define APP_SOFTOFF_CNT_TIME_10MS     0
#endif


#endif
#endif


