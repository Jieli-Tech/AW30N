
#ifndef _LIB_PROFILE_CFG_H_
#define _LIB_PROFILE_CFG_H_

#include "hid_app_config.h"
#include "btcontroller_modules.h"

#define BT_BTSTACK_CLASSIC                   BIT(0)
#define BT_BTSTACK_LE_ADV                    BIT(1)
#define BT_BTSTACK_LE                        BIT(2)

extern const int config_stack_modules;
#define STACK_MODULES_IS_SUPPORT(x)         (config_stack_modules & (x))

#define TCFG_BLE_DEMO_SELECT                (0xff)

#endif
