#ifndef __BT_BLE_H__
#define __BT_BLE_H__

// #include "bt_include/btstack_task.h"
// #include "bt_include/bluetooth.h"
// #include "btcontroller_modules.h"
#include "btcontroller_mode.h"
// #include "bt_include/le/le_common_define.h"
// #include "bt_include/le/ble_api.h"
#include "app_modules.h"
#include "app_config.h"
#include "btstack_task.h"
#include "bluetooth.h"
#include "btcontroller_modules.h"
#include "le/le_common_define.h"
#include "le/ble_api.h"
#include "btstack_typedef.h"
#include "third_party/common/ble_user.h"

// extern const char libs_debug; //打印总开关
#define CONFIG_DEBUG_ENABLE         DISABLE  //蓝牙总开关

//库打印
#ifdef CONFIG_RELEASE_ENABLE
#define LIB_DEBUG    0
#else
#define LIB_DEBUG    1
#endif

#if CONFIG_DEBUG_ENABLE
#define CONFIG_DEBUG_LIB(x)         ((x & LIB_DEBUG)) //bt 库打印
#else
#define CONFIG_DEBUG_LIB(x)         (0)
#endif

//BLE 性能测试 开关 （DUT测试）---仅在TESTE_BLE_EN模式
#define BLE_DUT_TEST  0

#include "app_modules.h"

//ble app select
#if (TESTE_BLE_EN || TRANS_DATA_HID_EN || TRANS_DATA_SPPLE_EN)
#define TRANS_DATA_EN               1//数传demo
#else
#define TRANS_DATA_EN               0//数传demo
#endif

#if CONFIG_APP_OTA_EN
#define RCSP_BTMATE_EN              1//rcsp app
#else
#define RCSP_BTMATE_EN              0//rcsp app
#endif
#define RCSP_ADV_EN                 0//not app
#define SMART_BOX_EN                0//not app
#define ANCS_CLIENT_EN              0//not app
#define LL_SYNC_EN                  0//not app
#define TUYA_DEMO_EN                0//not app
#define BLE_CLIENT_EN               0
#define TRANS_MULTI_BLE_EN          0
#define AI_APP_PROTOCOL             0//not app
#define BLE_WIRELESS_CLIENT_EN      0//not app
#define BLE_WIRELESS_SERVER_EN      0//not app
#define BLE_WIRELESS_1T1_TX_EN      0//无线麦1t1周期广播tx验证
#define BLE_WIRELESS_1T1_RX_EN      0//无线麦1t1周期广播rx验证
#define BLE_WIRELESS_1TN_TX_EN      0//not app
#define BLE_WIRELESS_1TN_RX_EN      0//not app
#define LE_AUDIO_EN                 0//not app
#define DEF_BLE_DEMO_MESH           0//not app
#define APP_ONLINE_DEBUG            0//not app

//*********************************************************************************//
//                                 BT 配置                                        //
//*********************************************************************************//
#define TCFG_USER_BLE_ENABLE            ENABLE_THIS_MOUDLE
#define TCFG_USER_EDR_ENABLE            DISABLE_THIS_MOUDLE

#define  SET_BLE_TX_POWER_LEVEL         (5)//0dBm
/* #define  SET_BLE_TX_POWER_LEVEL         (6)//4dBm */
/* #define  SET_BLE_TX_POWER_LEVEL         (7)//6dBm */


//lib_btstack_config.c
#define TCFG_BT_SUPPORT_AAC             DISABLE_THIS_MOUDLE
//profile
#define USER_SUPPORT_PROFILE_HID        0
#define USER_SUPPORT_PROFILE_HFP        0
#define USER_SUPPORT_PROFILE_SPP        0
#define USER_SUPPORT_PROFILE_HCRP       0

//*********************************************************************************//
//                                 USB 配置                                        //
//*********************************************************************************//
#if TCFG_PC_ENABLE
#define TCFG_BT_PC_ENABLE				    DISABLE_THIS_MOUDLE//蓝牙打开PC代码
#endif

//*********************************************************************************//
//                                  Other配置                                      //
//*********************************************************************************//
#define BT_FOR_APP_EN                     0
#define TCFG_POWER_MODE_QUIET_ENABLE      0

#define TCFG_SD0_SD1_USE_THE_SAME_HW      0
#define TCFG_KEEP_CARD_AT_ACTIVE_STATUS   0
#define TCFG_SDX_CAN_OPERATE_MMC_CARD     0
#define TCFG_POWER_MODE_QUIET_ENABLE      0
#define TCFG_POWER_MODE_QUIET_ENABLE      0

#define SUPPORT_TEST_BOX_BLE_MASTER_TEST_EN	0


typedef struct {
    u8 create_conn_mode;   //cli_creat_mode_e
    u8 bonding_flag;       //连接过后会绑定，默认快连，不搜索设备
    u8 compare_data_len;   //匹配信息长度
    const u8 *compare_data;//匹配信息，若是地址内容,由高到低位
    u8 filter_pdu_bitmap;     /*过滤指定的pdu包,不做匹配操作; bit map,event type*/
} client_match_cfg_t;

//搜索匹配连接方式
typedef enum {
    CLI_CREAT_BY_ADDRESS = 0,//指定地址创建连接
    CLI_CREAT_BY_NAME,//指定设备名称创建连接
    CLI_CREAT_BY_TAG,//匹配厂家标识创建连接
} cli_creat_mode_e;

enum {
    COMMON_EVENT_EDR_REMOTE_TYPE = 1,
    COMMON_EVENT_BLE_REMOTE_TYPE,
    COMMON_EVENT_SHUTDOWN_ENABLE,
    COMMON_EVENT_SHUTDOWN_DISABLE,
    COMMON_EVENT_MODE_DETECT,
};


//最大匹配的设备个数
#define CLIENT_MATCH_CONN_MAX    3

static const uint8_t profile_data[] = {
    //////////////////////////////////////////////////////
    //
    // 0x0001 PRIMARY_SERVICE  1800
    //
    //////////////////////////////////////////////////////
    0x0a, 0x00, 0x02, 0x00, 0x01, 0x00, 0x00, 0x28, 0x00, 0x18,

    /* CHARACTERISTIC,  2a00, READ | WRITE | DYNAMIC, */
    // 0x0002 CHARACTERISTIC 2a00 READ | WRITE | DYNAMIC
    0x0d, 0x00, 0x02, 0x00, 0x02, 0x00, 0x03, 0x28, 0x0a, 0x03, 0x00, 0x00, 0x2a,
    // 0x0003 VALUE 2a00 READ | WRITE | DYNAMIC
    0x08, 0x00, 0x0a, 0x01, 0x03, 0x00, 0x00, 0x2a,

    //////////////////////////////////////////////////////
    //
    // 0x0004 PRIMARY_SERVICE  ae30
    //
    //////////////////////////////////////////////////////
    0x0a, 0x00, 0x02, 0x00, 0x04, 0x00, 0x00, 0x28, 0x30, 0xae,

    /* CHARACTERISTIC,  ae01, WRITE_WITHOUT_RESPONSE | DYNAMIC, */
    // 0x0005 CHARACTERISTIC ae01 WRITE_WITHOUT_RESPONSE | DYNAMIC
    0x0d, 0x00, 0x02, 0x00, 0x05, 0x00, 0x03, 0x28, 0x04, 0x06, 0x00, 0x01, 0xae,
    // 0x0006 VALUE ae01 WRITE_WITHOUT_RESPONSE | DYNAMIC
    0x08, 0x00, 0x04, 0x01, 0x06, 0x00, 0x01, 0xae,

    /* CHARACTERISTIC,  ae02, NOTIFY, */
    // 0x0007 CHARACTERISTIC ae02 NOTIFY
    0x0d, 0x00, 0x02, 0x00, 0x07, 0x00, 0x03, 0x28, 0x10, 0x08, 0x00, 0x02, 0xae,
    // 0x0008 VALUE ae02 NOTIFY
    0x08, 0x00, 0x10, 0x00, 0x08, 0x00, 0x02, 0xae,
    // 0x0009 CLIENT_CHARACTERISTIC_CONFIGURATION
    0x0a, 0x00, 0x0a, 0x01, 0x09, 0x00, 0x02, 0x29, 0x00, 0x00,

    /* CHARACTERISTIC,  ae03, WRITE_WITHOUT_RESPONSE | DYNAMIC, */
    // 0x000a CHARACTERISTIC ae03 WRITE_WITHOUT_RESPONSE | DYNAMIC
    0x0d, 0x00, 0x02, 0x00, 0x0a, 0x00, 0x03, 0x28, 0x04, 0x0b, 0x00, 0x03, 0xae,
    // 0x000b VALUE ae03 WRITE_WITHOUT_RESPONSE | DYNAMIC
    0x08, 0x00, 0x04, 0x01, 0x0b, 0x00, 0x03, 0xae,

    /* CHARACTERISTIC,  ae04, NOTIFY, */
    // 0x000c CHARACTERISTIC ae04 NOTIFY
    0x0d, 0x00, 0x02, 0x00, 0x0c, 0x00, 0x03, 0x28, 0x10, 0x0d, 0x00, 0x04, 0xae,
    // 0x000d VALUE ae04 NOTIFY
    0x08, 0x00, 0x10, 0x00, 0x0d, 0x00, 0x04, 0xae,
    // 0x000e CLIENT_CHARACTERISTIC_CONFIGURATION
    0x0a, 0x00, 0x0a, 0x01, 0x0e, 0x00, 0x02, 0x29, 0x00, 0x00,

    /* CHARACTERISTIC,  ae05, INDICATE, */
    // 0x000f CHARACTERISTIC ae05 INDICATE
    0x0d, 0x00, 0x02, 0x00, 0x0f, 0x00, 0x03, 0x28, 0x20, 0x10, 0x00, 0x05, 0xae,
    // 0x0010 VALUE ae05 INDICATE
    0x08, 0x00, 0x20, 0x00, 0x10, 0x00, 0x05, 0xae,
    // 0x0011 CLIENT_CHARACTERISTIC_CONFIGURATION
    0x0a, 0x00, 0x0a, 0x01, 0x11, 0x00, 0x02, 0x29, 0x00, 0x00,

    /* CHARACTERISTIC,  ae10, READ | WRITE | DYNAMIC, */
    // 0x0012 CHARACTERISTIC ae10 READ | WRITE | DYNAMIC
    0x0d, 0x00, 0x02, 0x00, 0x12, 0x00, 0x03, 0x28, 0x0a, 0x13, 0x00, 0x10, 0xae,
    // 0x0013 VALUE ae10 READ | WRITE | DYNAMIC
    0x08, 0x00, 0x0a, 0x01, 0x13, 0x00, 0x10, 0xae,


    //////////////////////////////////////////////////////
    //
    // 0x0040 PRIMARY_SERVICE  ae3a
    //
    //////////////////////////////////////////////////////
    0x0a, 0x00, 0x02, 0x00, 0x40, 0x00, 0x00, 0x28, 0x3a, 0xae,

    /* CHARACTERISTIC,  ae3b, WRITE_WITHOUT_RESPONSE | DYNAMIC, */
    // 0x0041 CHARACTERISTIC ae3b WRITE_WITHOUT_RESPONSE | DYNAMIC
    0x0d, 0x00, 0x02, 0x00, 0x41, 0x00, 0x03, 0x28, 0x04, 0x42, 0x00, 0x3b, 0xae,
    // 0x0042 VALUE ae3b WRITE_WITHOUT_RESPONSE | DYNAMIC
    0x08, 0x00, 0x04, 0x01, 0x42, 0x00, 0x3b, 0xae,

    /* CHARACTERISTIC,  ae3c, NOTIFY, */
    // 0x0043 CHARACTERISTIC ae3c NOTIFY
    0x0d, 0x00, 0x02, 0x00, 0x43, 0x00, 0x03, 0x28, 0x10, 0x44, 0x00, 0x3c, 0xae,
    // 0x0044 VALUE ae3c NOTIFY
    0x08, 0x00, 0x10, 0x00, 0x44, 0x00, 0x3c, 0xae,
    // 0x0045 CLIENT_CHARACTERISTIC_CONFIGURATION
    0x0a, 0x00, 0x0a, 0x01, 0x45, 0x00, 0x02, 0x29, 0x00, 0x00,

    //////////////////////////////////////////////////////
    //
    // 0x0046 PRIMARY_SERVICE  1801
    //
    //////////////////////////////////////////////////////
    0x0a, 0x00, 0x02, 0x00, 0x46, 0x00, 0x00, 0x28, 0x01, 0x18,

    /* CHARACTERISTIC,  2a05, INDICATE, */
    // 0x0047 CHARACTERISTIC 2a05 INDICATE
    0x0d, 0x00, 0x02, 0x00, 0x47, 0x00, 0x03, 0x28, 0x20, 0x48, 0x00, 0x05, 0x2a,
    // 0x0048 VALUE 2a05 INDICATE
    0x08, 0x00, 0x20, 0x00, 0x48, 0x00, 0x05, 0x2a,
    // 0x0049 CLIENT_CHARACTERISTIC_CONFIGURATION
    0x0a, 0x00, 0x0a, 0x01, 0x49, 0x00, 0x02, 0x29, 0x00, 0x00,

#if TRANS_DATA_EN
    //////////////////////////////////////////////////////
    //
    // 0x0004 PRIMARY_SERVICE  ae00
    //
    //////////////////////////////////////////////////////
    0x0a, 0x00, 0x02, 0x00, 0x80, 0x00, 0x00, 0x28, 0x00, 0xae,

    /* CHARACTERISTIC,  ae01, WRITE_WITHOUT_RESPONSE | DYNAMIC, */
    // 0x0040 CHARACTERISTIC ae01 WRITE_WITHOUT_RESPONSE | DYNAMIC
    0x0d, 0x00, 0x02, 0x00, 0x81, 0x00, 0x03, 0x28, 0x04, 0x82, 0x00, 0x01, 0xae,
    // 0x0041 VALUE ae01 WRITE_WITHOUT_RESPONSE | DYNAMIC
    0x08, 0x00, 0x04, 0x01, 0x82, 0x00, 0x01, 0xae,

    /* CHARACTERISTIC,  ae02, NOTIFY, */
    // 0x0042 CHARACTERISTIC ae02 NOTIFY
    0x0d, 0x00, 0x02, 0x00, 0x83, 0x00, 0x03, 0x28, 0x10, 0x84, 0x00, 0x02, 0xae,
    // 0x0043 VALUE ae02 NOTIFY
    0x08, 0x00, 0x10, 0x00, 0x84, 0x00, 0x02, 0xae,
    // 0x0044 CLIENT_CHARACTERISTIC_CONFIGURATION
    0x0a, 0x00, 0x0a, 0x01, 0x85, 0x00, 0x02, 0x29, 0x00, 0x00,
#endif

    // END
    0x00, 0x00,
};
//
// characteristics <--> handles
//
#if TRANS_DATA_EN
#define ATT_CHARACTERISTIC_ae01_02_VALUE_HANDLE 0x0082
#define ATT_CHARACTERISTIC_ae02_02_VALUE_HANDLE 0x0084
#define ATT_CHARACTERISTIC_ae02_02_CLIENT_CONFIGURATION_HANDLE 0x0085
#endif

#define ATT_CHARACTERISTIC_2a00_01_VALUE_HANDLE 0x0003
#define ATT_CHARACTERISTIC_ae01_01_VALUE_HANDLE 0x0006
#define ATT_CHARACTERISTIC_ae02_01_VALUE_HANDLE 0x0008
#define ATT_CHARACTERISTIC_ae02_01_CLIENT_CONFIGURATION_HANDLE 0x0009
#define ATT_CHARACTERISTIC_ae03_01_VALUE_HANDLE 0x000b
#define ATT_CHARACTERISTIC_ae04_01_VALUE_HANDLE 0x000d
#define ATT_CHARACTERISTIC_ae04_01_CLIENT_CONFIGURATION_HANDLE 0x000e
#define ATT_CHARACTERISTIC_ae05_01_VALUE_HANDLE 0x0010
#define ATT_CHARACTERISTIC_ae05_01_CLIENT_CONFIGURATION_HANDLE 0x0011
#define ATT_CHARACTERISTIC_ae10_01_VALUE_HANDLE 0x0013

#define ATT_CHARACTERISTIC_ae3b_01_VALUE_HANDLE 0x0042
#define ATT_CHARACTERISTIC_ae3c_01_VALUE_HANDLE 0x0044
#define ATT_CHARACTERISTIC_ae3c_01_CLIENT_CONFIGURATION_HANDLE 0x0045
#define ATT_CHARACTERISTIC_2a05_01_VALUE_HANDLE 0x0048
#define ATT_CHARACTERISTIC_2a05_01_CLIENT_CONFIGURATION_HANDLE 0x0049

#define ATT_SEND_DATA_ID_1 ATT_CHARACTERISTIC_ae02_01_VALUE_HANDLE
#define ATT_SEND_DATA_ID_2 ATT_CHARACTERISTIC_ae04_01_VALUE_HANDLE
#define ATT_SEND_DATA_ID_3 ATT_CHARACTERISTIC_ae05_01_VALUE_HANDLE
#define ATT_SEND_DATA_RCSP ATT_CHARACTERISTIC_ae02_02_VALUE_HANDLE

//主机事件
typedef enum {
    CLI_EVENT_MATCH_DEV = 1,//搜索到匹配的设备
    CLI_EVENT_CONNECTED, //设备连接成功
    CLI_EVENT_DISCONNECT,//设备连接断开
    CLI_EVENT_MATCH_UUID,//搜索到匹配的UUID
    CLI_EVENT_SEARCH_PROFILE_COMPLETE, //搜索profile服务结束
    CLI_EVENT_CONNECTION_UPDATE,//设备连接参数更新成功
} le_client_event_e;


typedef struct {
    //服务16bit uuid,非0 代表uuid16格式，0--代表是uuid128格式,services_uuid128
    u16 services_uuid16;
    //服务16bit uuid,非0 代表uuid16格式，0--代表是uuid128格式,characteristic_uuid128
    u16 characteristic_uuid16;
    u8  services_uuid128[16];
    u8  characteristic_uuid128[16];
    u16 opt_type; //属性
    u8 read_long_enable: 1; //en
    u8 read_report_reference: 1; //en
    u8 res_bits: 6; //
} target_uuid_t;


//搜索操作记录的 handle
#define OPT_HANDLE_MAX   16
typedef struct {
    //匹配的UUID
    target_uuid_t *search_uuid;
    //可操作的handle
    u16 value_handle;
} opt_handle_t;

//最大匹配的设备个数
#define CLIENT_MATCH_CONN_MAX    3



typedef struct {
    //搜索匹配信息
    const client_match_cfg_t *match_dev_cfg[CLIENT_MATCH_CONN_MAX];
    //加密保定配置 0 or 1
    u8 security_en;
    //搜索服务的个数
    u8 search_uuid_cnt; // <= OPT_HANDLE_MAX
    //搜索服务
    const target_uuid_t *search_uuid_table;
    //回调处理接收到的 notify or indicate 数据
    void (*report_data_callback)(att_data_report_t *data_report, target_uuid_t *search_uuid);
    //主机一些事件回调处理
    void (*event_callback)(le_client_event_e event, u8 *packet, int size);
} client_conn_cfg_t;

#define ble_clock_init()                                    \
                        SFR(JL_CLOCK->PRP_CON2, 0, 2, 1); \
                        SFR(JL_CLOCK->PRP_CON2, 2, 2, 2); \
                        SFR(JL_CLOCK->PRP_CON2, 4, 2, 2); \
                        SFR(JL_CLOCK->PRP_CON2, 8, 1, 1); \
                        asm("csync"); \
                        bt_pll_para(48000000, 48000000, 0, 0); \
                        bt_max_pwr_set(10, 5, 8, SET_BLE_TX_POWER_LEVEL);//0dBm

int ble_priv_audio_send_api(u8 *data, u32 len);
void ble_priv_audio_recv_cb_register(int (*callback_func)(u8 *, u16));
#if TESTE_BLE_EN
int app_ble_send_data_api(u8 *data, u32 len);
void app_ble_recv_callback_register(int (*callback_func)(u8 *buf, u16 len));
#endif

static const char *const phy_result[] = {
    "None",
    "1M",
    "2M",
    "Coded",
};

int get_buffer_vaild_len(void *priv);
// ble_slave
void bt_ble_slave_init(void);
void bt_ble_slave_exit(void);
int bt_ble_slave_send_api(u8 *data, u16 len);
void bt_ble_slave_recv_register(void *priv, int (*callback_func)(void *priv, u8 *buf, u16 len));
void ble_slave_module_enable(u8 en);
void bt_ble_slave_adv_enable(u8 enable);
void ble_slave_profile_init(void);
// ble_master
void bt_ble_master_init(void);
void bt_ble_master_exit(void);
int bt_ble_master_send_api(u8 *data, u16 len);
void bt_ble_master_recv_register(void *priv, int (*callback_func)(void *priv, u8 *buf, u16 len));
void ble_master_module_enable(u8 en);
void bt_ble_master_adv_enable(u8 enable);
void ble_master_profile_init(void);

typedef struct _vble_smpl_role_ops {
    char *name;
    void (*init)(void);
    void (*exit)(void);
    int (*send)(u8 *data, u16 len);
    void (*recv_cb_register)(void *priv, int (*cb_func)(void *priv, u8 *, u16));
    void (*module_enable)(u8 en);
    void (*adv_enable)(u8 enable);
    void (*profile_init)(void);
    u32(*get_ble_status)(void);
    u32(*update_conn_interval)(u16 interval, u16 set_timeout);
} vble_smpl_role_ops;

#endif
