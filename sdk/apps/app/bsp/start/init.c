
#pragma bss_seg(".init.data.bss")
#pragma data_seg(".init.data")
#pragma const_seg(".init.text.const")
#pragma code_seg(".init.text")
#pragma str_literal_override(".init.text.const")

#include "init.h"
#include "tick_timer_driver.h"
#include "device.h"
#include "vfs.h"
#include "msg.h"
#include "my_malloc.h"
#include "init_app.h"
#include "audio.h"
#include "sys_timer.h"
#include "app_modules.h"
#include "flash_init.h"

#if TESTBOX_UART_UPDATE_EN
#include "testbox_uart_update.h"
#endif

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[init]"
#include "log.h"

void system_init(void)
{
    /* my_malloc_init(); */
#if SYS_TIMER_EN
    /* sys_timer_init(); */
#endif
    tick_timer_init();
    message_init();
    /* app_system_init(); */
    devices_init_api();
    flash_system_init();

#if UPDATE_V2_EN
    //升级初始化
    int app_update_init(void);
    app_update_init();

#if TESTBOX_UART_UPDATE_EN
    /* 测试盒串口升级 */
    testbox_uart_update_init();
#endif

#if CONFIG_APP_OTA_EN
    extern void rcsp_init();
    rcsp_init();
#endif

#endif
}

