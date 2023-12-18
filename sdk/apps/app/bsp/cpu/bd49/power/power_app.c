#include "asm/power_interface.h"

//-----------------------------------------------------------------------------------------------------------------------
/* config
 */
#include "gpio.h"
#include "uart.h"
#include "app_config.h"

#define CONFIG_UART_DEBUG_ENABLE	UART_DEBUG
#define CONFIG_UART_DEBUG_PORT		TCFG_UART_TX_PORT

#if (CONFIG_UART_DEBUG_ENABLE && ((CONFIG_UART_DEBUG_PORT == IO_PORT_DP) || (CONFIG_UART_DEBUG_PORT == IO_PORT_DM)))
const u32 usb_bk = 1;

#else
const u32 usb_bk = 0;

#endif

//-------------------------------------------------------------------
/*调试pdown进不去的场景，影响低功耗流程
 * 打印蓝牙和系统分别可进入低功耗的时间(msec)
 * 打印当前哪些模块处于busy,用于蓝牙已经进入sniff但系统无法进入低功耗的情况，如果usr_timer处于busy则会打印对应的func地址
 */
const char debug_is_idle = 0;

//-------------------------------------------------------------------
/* 调试快速起振信息，不影响低功耗流程
 */
const bool pdebug_xosc_resume = 0;

//-------------------------------------------------------------------
/* 调试低功耗流程
 */
//出pdown打印信息，不影响低功耗流程
const bool pdebug_pdown_info = 0;

//使能串口调试低功耗，在pdown、soff模式保持串口，配合打开log_debug\pdebug_uart_pdown
const u32 pdebug_uart_lowpower = 0;
const u32 pdebug_uart_port = CONFIG_UART_DEBUG_PORT;

//使能串口putbyte调试pdown流程(不影响pdown流程)
const bool pdebug_putbyte_pdown = 0;

extern const char libs_debug;
extern void get_vir_rtc_status(void);

void power_early_flowing()
{
    PORT_TABLE(g);

    init_boot_rom();

    printf("get_boot_rom(): %d", get_boot_rom());

    p33_mclr_sw(0);
    // 默认关闭长按复位0，由key_driver配置
    gpio_longpress_pin0_reset_config(IO_PORTA_03, 0, 0, 1, 1);
    //长按复位1默认配置8s，写保护
    //gpio_longpress_pin1_reset_config(IO_LDOIN_DET, 1, 8, 1);

    if (libs_debug == TRUE) {
        PORT_PROTECT(CONFIG_UART_DEBUG_PORT);

    }

    get_vir_rtc_status();

    power_early_init((u32)gpio_config);
}

//early_initcall(_power_early_init);


int power_later_init()
{
    pmu_trim(0, 0);

    return 0;
}

//late_initcall(power_later_init);

