#ifndef __JL_BLUETOOTH_H__
#define __JL_BLUETOOTH_H__

#include "bt_ble.h"
#include "app_modules.h"

enum {
    ANDROID,
    APPLE_IOS,
};

enum {
    RCSP_BLE,
    RCSP_SPP,
    RCSP_HID,
};

#define     RCSP_SDK_TYPE_AC690X                        0x0
#define     RCSP_SDK_TYPE_AC692X                        0x1
#define     RCSP_SDK_TYPE_AC693X                        0x2
#define     RCSP_SDK_TYPE_AC695X                        0x3
#define     RCSP_SDK_TYPE_AC697X                        0x4
#define     RCSP_SDK_TYPE_AC696X                        0x5
#define     RCSP_SDK_TYPE_AC696X_TWS                    0x6
#define     RCSP_SDK_TYPE_AC695X_WATCH                  0x8
#define     RCSP_SDK_TYPE_AC701N_WATCH                  0x9
#define     RCSP_SDK_TYPE_MANIFEST_EARPHONE             0xA
#define     RCSP_SDK_TYPE_MANIFEST_SOUNDBOX             0xB

// RCSP命令码
#define    JL_OPCODE_DATA                                           0x01
#define    JL_OPCODE_GET_TARGET_FEATURE                             0x03
#define    JL_OPCODE_SYS_INFO_GET                                   0x07
#define    JL_OPCODE_SWITCH_DEVICE                                  0x0b

#define     SDK_TYPE_AC690X     0x0
#define     SDK_TYPE_AC692X     0x1
#define     SDK_TYPE_AC693X     0x2
#define     SDK_TYPE_AC695X     0x3
#define     SDK_TYPE_AC697X     0x4

#if   (defined CONFIG_CPU_BR21)
#define     RCSP_SDK_TYPE       SDK_TYPE_AC692X
#elif (defined CONFIG_CPU_BR22)
#define     RCSP_SDK_TYPE       SDK_TYPE_AC693X
#elif (defined CONFIG_CPU_BR23)
#define     RCSP_SDK_TYPE       SDK_TYPE_AC695X
#elif (defined CONFIG_CPU_BR30)
#define     RCSP_SDK_TYPE       SDK_TYPE_AC697X
#else
#define     RCSP_SDK_TYPE       SDK_TYPE_AC693X
#endif

#define RCSP_BLE_DEAL_OK		0
//#define RCSP_BTMATE_EN			0

void rcsp_init();
void rcsp_exit(void);
void JL_ble_disconnect(void);
void set_curr_update_type(u8 type);
u8 get_curr_device_type(void);
void rcsp_resume_do(void);
int rcsp_get_msg(int len, int *msg);
u8 get_rcsp_connect_status(void);
void app_update_start(int msg);
u8 get_rcsp_support_new_reconn_flag(void);
void rcsp_dev_select(u8 type);

#endif
