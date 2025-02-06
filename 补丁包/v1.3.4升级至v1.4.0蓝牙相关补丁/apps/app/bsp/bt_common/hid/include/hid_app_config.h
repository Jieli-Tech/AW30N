#ifndef HID_APP_CONFIG_H
#define HID_APP_CONFIG_H

#include "bt_ble.h"

#if (TRANS_DATA_HID_EN)
//app case 选择,只选1,要配置对应的board_config.h
#define CONFIG_APP_KEYBOARD                 1//hid按键 ,default case
#define CONFIG_APP_KEYFOB                   0//自拍器,  board_ac6368a,board_6318,board_6379b
#define CONFIG_APP_MOUSE_SINGLE             0//单模切换
#define CONFIG_APP_MOUSE_DUAL               0//同时开双模
#define CONFIG_APP_STANDARD_KEYBOARD        0//标准HID键盘,board_ac6351d
#define CONFIG_APP_KEYPAGE                  0//翻页器
#define CONFIG_APP_GAMEBOX                  0//吃鸡王座
#define CONFIG_APP_REMOTE_CONTROL           0//语音遥控
#define CONFIG_APP_IDLE                     0//IDLE

#define CONFIG_IOT_ENABLE                   1//配置蓝牙接外部APP的应用

//edr sniff模式选择; sniff参数需要调整,移植可具体参考app_keyboard.c
#if CONFIG_APP_MOUSE_SINGLE || CONFIG_APP_MOUSE_DUAL || CONFIG_APP_STANDARD_KEYBOARD || CONFIG_APP_REMOTE_CONTROL //|| CONFIG_APP_KEYBOARD
#define SNIFF_MODE_RESET_ANCHOR             1//键盘鼠标sniff模式,固定小周期发包,多按键响应快
#else
#define SNIFF_MODE_RESET_ANCHOR             0//待机固定500ms sniff周期,待机功耗较低,按键唤醒有延时
#endif

#define CFG_APP_RUN_BY_WHILE                1//设置app_core跑裸机

//add in bt_ble.h
#if 1
#define CONFIG_HOGP_COMMON_ENABLE          1 //公共的hogp
#ifdef CUT_RAM_BUF_ENABLE
#define CONFIG_SLAVE_ADD_LE_SCAN           1 //在从机基础上添加scan功能
#else
#define CONFIG_SLAVE_ADD_LE_SCAN           0 //在从机基础上添加scan功能
#endif

//蓝牙BLE配置
#define DOUBLE_BT_SAME_NAME                0 //同名字
#define CONFIG_BT_GATT_COMMON_ENABLE       1 //配置使用gatt公共模块
#define CONFIG_BT_SM_SUPPORT_ENABLE        1 //配置是否支持加密
#define CONFIG_BT_GATT_CLIENT_NUM          0 //配置主机client个数(app not support)
#define CONFIG_BT_GATT_SERVER_NUM          1 //配置从机server个数
#define CONFIG_BT_GATT_CONNECTION_NUM      (CONFIG_BT_GATT_SERVER_NUM + CONFIG_BT_GATT_CLIENT_NUM) //配置连接个数

#ifdef CUT_RAM_BUF_ENABLE
#define CONFIG_BLE_HIGH_SPEED              0 //BLE提速模式: 使能DLE+2M, payload要匹配pdu的包长
#else
#define CONFIG_BLE_HIGH_SPEED              1 //BLE提速模式: 使能DLE+2M, payload要匹配pdu的包长
#endif
#define CONFIG_BLE_CONNECT_SLOT            0 //BLE高回报率设置, 支持私有协议
#define TCFG_HID_AUTO_SHUTDOWN_TIME       (0 * 60)      //HID无操作自动关机(单位：秒)
#endif


#if CONFIG_SLAVE_ADD_LE_SCAN
#define CONFIG_BT_GATT_COMMON_GET_ADV_TEPORT_ENABLE       1 //配置直接获取adv report使能,针对不开gatt client情况下
#else
#define CONFIG_BT_GATT_COMMON_GET_ADV_TEPORT_ENABLE       0 //
#endif

#ifndef BT_NV_RAM_SIZE
#ifdef CUT_RAM_BUF_ENABLE
#define BT_NV_RAM_SIZE                    (0x2800)  //bt nv malloc堆的大小
#else
#define BT_NV_RAM_SIZE                    (0x2800)  //bt nv malloc堆的大小
#endif
#endif

//需要app(BLE)升级要开一下宏定义
#if CONFIG_APP_OTA_EN
#define RCSP_UPDATE_EN                    1
#define UPDATE_MD5_ENABLE                 0
#else
#define RCSP_UPDATE_EN                    0
#define UPDATE_MD5_ENABLE                 0
#endif


//APP应用默认配置
//TODO
// #define TCFG_AEC_ENABLE                     1

// #define TCFG_MEDIA_LIB_USE_MALLOC		    1

// #include "board_config.h"

// #include "usb_common_def.h"

#include "btcontroller_mode.h"

#include "user_cfg_id.h"

#define APP_PRIVATE_PROFILE_CFG



#ifdef CONFIG_SDFILE_ENABLE
#define SDFILE_DEV				"sdfile"
#define SDFILE_MOUNT_PATH     	"mnt/sdfile"

#if (USE_SDFILE_NEW)
#define SDFI[MaP*LE_APP_ROOT_PATH       	SDFILE_MOUNT_PATH"/app/"  //app分区
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
//                                  低功耗配置                                     //
//*********************************************************************************//
#define TCFG_LOWPOWER_LOWPOWER_SEL      ENABLE


#if (CONFIG_BT_MODE == BT_NORMAL)
//enable dut mode,need disable sleep(TCFG_LOWPOWER_LOWPOWER_SEL = 0)
// #define BLE_DUT_TEST                  0
#if BLE_DUT_TEST
#undef  TCFG_LOWPOWER_LOWPOWER_SEL
#define TCFG_LOWPOWER_LOWPOWER_SEL    0

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

#undef TCFG_PC_ENABLE
#define TCFG_PC_ENABLE				  0     //PC模块

#undef TCFG_UDISK_ENABLE
#define TCFG_UDISK_ENABLE			  0

#undef POWERDOWN_UDISK_MODE_EN
#define POWERDOWN_UDISK_MODE_EN		  0

#undef APP_SOFTOFF_CNT_TIME_10MS
#define APP_SOFTOFF_CNT_TIME_10MS     0

#endif




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
#endif
#endif


