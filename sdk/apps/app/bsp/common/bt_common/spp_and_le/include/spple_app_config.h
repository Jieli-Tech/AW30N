#ifndef SPPLE_APP_CONFIG_H
#define SPPLE_APP_CONFIG_H

#include "bt_ble.h"

#if (TRANS_DATA_SPPLE_EN)

#define TCFG_MEDIA_LIB_USE_MALLOC		    1
//apps example 选择,只能选1个,要配置对应的board_config.h
#define CONFIG_APP_SPP_LE                 0 //SPP + LE or LE's client
#define CONFIG_APP_MULTI                  0 //蓝牙LE多连 + spp
#define CONFIG_APP_DONGLE                 1 //usb + 蓝牙(ble 主机),PC hid设备  目前USB仅支持单设备
#define CONFIG_APP_CENTRAL                0 //ble client,中心设备
#define CONFIG_APP_LL_SYNC                0 //腾讯连连
#define CONFIG_APP_BEACON                 0 //蓝牙BLE ibeacon
#define CONFIG_APP_NONCONN_24G            0 //2.4G 非连接收发
#define CONFIG_APP_TUYA                   0 //涂鸦协议
#define CONFIG_APP_AT_COM                 0 //AT com HEX格式命令
#define CONFIG_APP_AT_CHAR_COM            0 //AT com 字符串格式命令
#define CONFIG_APP_IDLE                   0 //空闲任务
#define CONFIG_APP_CONN_24G               0 //基于BLE的2.4g,板级只需要开BLE
#define CONFIG_APP_HILINK                 0 //华为协议
#define CONFIG_APP_ELECTROCAR             0 //电车项目,注意将板级处ADKEY以及别的IO占用失能, 关闭低功耗:TCFG_LOWPOWER_LOWPOWER_SEL设置为0
// #define LL_SYNC_EN                        CONFIG_APP_LL_SYNC //
// #define TUYA_DEMO_EN                      CONFIG_APP_TUYA

#define CFG_APP_RUN_BY_WHILE              1//设置app_core跑裸机

//edr sniff config
#define SNIFF_MODE_RESET_ANCHOR           0
//V5.0 扩展广播/扫描使能
#define CONFIG_BT_EXT_ADV_MODE            0

//BLE做主机，使能是否支持搜索连接JL的测试盒
#define SUPPORT_TEST_BOX_BLE_MASTER_TEST_EN	   1

#if CONFIG_APP_SPP_LE
//配置双模同名字，同地址
#define DOUBLE_BT_SAME_NAME                0 //同名字
#define DOUBLE_BT_SAME_MAC                 0 //同地址
#define CONFIG_APP_SPP_LE_TO_IDLE          0 //SPP_AND_LE To IDLE Use
#define CONFIG_BLE_HIGH_SPEED              0 //BLE提速模式: 使能DLE+2M, payload要匹配pdu的包长

//蓝牙BLE配置
#define CONFIG_BT_GATT_COMMON_ENABLE       1 //配置使用gatt公共模块
#define CONFIG_BT_SM_SUPPORT_ENABLE        0 //配置是否支持加密
#define CONFIG_BT_GATT_CLIENT_NUM          0 //配置主机client个数 (app not support,应用不支持使能)
#define CONFIG_BT_GATT_SERVER_NUM          1 //配置从机server个数
#define CONFIG_BT_GATT_CONNECTION_NUM      (CONFIG_BT_GATT_SERVER_NUM + CONFIG_BT_GATT_CLIENT_NUM) //配置连接个数

//BLE 从机扩展搜索对方服务功能,需要打开GATT CLIENT
#if CONFIG_BT_GATT_CLIENT_NUM
#define TRANS_CLIENT_SEARCH_PROFILE_ENABLE  1/*配置模块搜索指定的服务*/

#if !TRANS_CLIENT_SEARCH_PROFILE_ENABLE && CONFIG_BT_SM_SUPPORT_ENABLE /*定制搜索ANCS&AMS服务*/
#define TRANS_ANCS_EN                       1/*配置搜索主机的ANCS 服务,要开配对绑定*/
#define TRANS_AMS_EN                        0/*配置搜索主机的ANCS 服务,要开配对绑定*/
#endif
#endif//#if CONFIG_BT_GATT_CLIENT_NUM

#elif CONFIG_APP_DONGLE
/*默认做搜索设备匹配名字再发起连接 inquery + page*/
#define EDR_EMITTER_EN                     0 //蓝牙(edr主机)

#if EDR_EMITTER_EN
/*不做搜索匹配 + 连接，只开放可连接，等待对方连接*/
#define EDR_EMITTER_PAGESCAN_ONLY          0 /**/
#endif

#define CONFIG_BT_GATT_COMMON_ENABLE       1
#define CONFIG_BT_SM_SUPPORT_ENABLE        1 //配置是否支持加密
#define CONFIG_BT_GATT_CLIENT_NUM          1 //设置主机个数1~2,(dongle可连接蓝牙BLE设备的个数,=2时会多注册1个usb设备)
#define CONFIG_BT_COMPOSITE_EQUIPMENT      0
#define CONFIG_BT_GATT_SERVER_NUM          0 //
#define CONFIG_BT_GATT_CONNECTION_NUM      (CONFIG_BT_GATT_SERVER_NUM + CONFIG_BT_GATT_CLIENT_NUM) //
#define CONFIG_BLE_CONNECT_SLOT            0 //BLE高回报率设置, 支持私有协议
#define CONFIG_BLE_HIGH_SPEED              1 //BLE提速模式: 使能DLE+2M, payload要匹配pdu的包长

#else
#define CONFIG_BT_GATT_COMMON_ENABLE       0
#define CONFIG_BT_SM_SUPPORT_ENABLE        0
#define CONFIG_BT_GATT_CLIENT_NUM          0
#define CONFIG_BT_GATT_SERVER_NUM          0
#define CONFIG_BT_GATT_CONNECTION_NUM      0

#endif


#if CONFIG_BT_GATT_CONNECTION_NUM > 8
#error "SUPPORT MAX IS 8 !!!"
#endif

#ifndef EDR_EMITTER_EN
#define EDR_EMITTER_EN                     0 //蓝牙(edr主机)
#endif

// #include "board_config.h"
//
// #include "usb_common_def.h"

#include "btcontroller_mode.h"

#include "user_cfg_id.h"

#define APP_PRIVATE_PROFILE_CFG

#if CONFIG_BT_EXT_ADV_MODE
#define APP_TO_ALLOW_EXT_ADV
#endif

#if (CONFIG_BT_MODE == BT_NORMAL)
//enable dut mode,need disable sleep(TCFG_LOWPOWER_LOWPOWER_SEL = 0)
#define TCFG_NORMAL_SET_DUT_MODE                  0
#if TCFG_NORMAL_SET_DUT_MODE
#undef  TCFG_LOWPOWER_LOWPOWER_SEL
#define TCFG_LOWPOWER_LOWPOWER_SEL                0

#endif //#if TCFG_NORMAL_SET_DUT_MODE

#else

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

#undef TCFG_UART0_ENABLE
#define TCFG_UART0_ENABLE					DISABLE_THIS_MOUDLE

#endif


#define BT_FOR_APP_EN                     0

//需要app(BLE)升级要开一下宏定义
#if CONFIG_APP_OTA_EN
#define RCSP_UPDATE_EN                    1
#define UPDATE_MD5_ENABLE                 0
#else
#define RCSP_UPDATE_EN                    0
#define UPDATE_MD5_ENABLE                 0
#endif


#ifdef CONFIG_SDFILE_ENABLE
#define SDFILE_DEV				"sdfile"
#define SDFILE_MOUNT_PATH     	"mnt/sdfile"

#if (USE_SDFILE_NEW)
#define SDFILE_APP_ROOT_PATH       	SDFILE_MOUNT_PATH"/app/"  //app分区
#define SDFILE_RES_ROOT_PATH       	SDFILE_MOUNT_PATH"/res/"  //资源文件分区
#else
#define SDFILE_RES_ROOT_PATH       	SDFILE_MOUNT_PATH"/C/"
#endif

#endif
#define CONFIG_BT_RX_BUFF_SIZE  (0)
#define CONFIG_BT_TX_BUFF_SIZE  (0)

#if (CONFIG_BT_MODE != BT_NORMAL)
////bqb 如果测试3M tx buf 最好加大一点
#undef  CONFIG_BT_TX_BUFF_SIZE
#define CONFIG_BT_TX_BUFF_SIZE  (6 * 1024)

#endif
#define BT_NORMAL_HZ	            CONFIG_BT_NORMAL_HZ
//*********************************************************************************//
//                                 时钟切换配置                                    //
//*********************************************************************************//

#define BT_NORMAL_HZ	            CONFIG_BT_NORMAL_HZ
#define BT_CONNECT_HZ               CONFIG_BT_CONNECT_HZ

#define BT_A2DP_HZ	        	    CONFIG_BT_A2DP_HZ
#define BT_TWS_DEC_HZ	        	CONFIG_TWS_DEC_HZ

//#define MUSIC_DEC_CLOCK			    CONFIG_MUSIC_DEC_CLOCK
//#define MUSIC_IDLE_CLOCK		    CONFIG_MUSIC_IDLE_CLOCK

#define BT_CALL_HZ		            CONFIG_BT_CALL_HZ
#define BT_CALL_ADVANCE_HZ          CONFIG_BT_CALL_ADVANCE_HZ
#define BT_CALL_16k_HZ	            CONFIG_BT_CALL_16k_HZ
#define BT_CALL_16k_ADVANCE_HZ      CONFIG_BT_CALL_16k_ADVANCE_HZ

//*********************************************************************************//
//                                 升级配置                                        //
//*********************************************************************************//
#if (defined(CONFIG_CPU_BR30))
//升级LED显示使能
//#define UPDATE_LED_REMIND
//升级提示音使能
//#define UPDATE_VOICE_REMIND
#endif

#if (defined(CONFIG_CPU_BR23) || defined(CONFIG_CPU_BR25))
//升级IO保持使能
//#define DEV_UPDATE_SUPPORT_JUMP           //目前只有br23\br25支持
#endif

#if (defined(CONFIG_CPU_BR23) || defined(CONFIG_CPU_BR25) || defined(CONFIG_CPU_BD29) || defined(CONFIG_CPU_BD19) || defined(CONFIG_CPU_BR30) || defined(CONFIG_CPU_BR34) || defined(CONFIG_CPU_BD47) || defined(CONFIG_CPU_Bd49))
#define USER_UART_UPDATE_ENABLE           0//用于客户开发上位机或者多MCU串口升级方案,请确保串口通信前已经退出powerdown,需要在对应的board添加初始化和唤醒口配置参考board_ac632n_demo.c
#define XM_MMA_EN           0
#define UART_UPDATE_SLAVE	0
#define UART_UPDATE_MASTER	1

//配置串口升级IO
#define UART_UPDATE_RX_PORT               IO_PORTA_02
#define UART_UPDATE_TX_PORT               IO_PORTA_03

//配置串口升级的角色
#define UART_UPDATE_ROLE	UART_UPDATE_SLAVE

#if USER_UART_UPDATE_ENABLE
#undef TCFG_CHARGESTORE_ENABLE
#undef TCFG_TEST_BOX_ENABLE
#define TCFG_CHARGESTORE_ENABLE				DISABLE_THIS_MOUDLE       //用户串口升级也使用了UART1
#endif

#endif  //USER_UART_UPDATE_ENABLE

#define FLOW_CONTROL           0  //AT 字符串口流控, 目前只有br30做了测试


#endif
#endif

