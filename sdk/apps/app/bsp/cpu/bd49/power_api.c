#include "asm/power_interface.h"
#include "sys_memory.h"
#include "adc_api.h"
#include "audio.h"
#include "gpio_hw.h"
#include "app_config.h"

#define PORT_WKUP_FILTER PORT_FLT_1ms

void sys_softoff()
{
    //关机前做预擦除动作
    adc_hw_uninit();
    audio_off();
    sysmem_pre_erase_api();
    power_set_soft_poweroff();
}

void vPortSuppressTicksAndSleep(u32 usec);
void sys_power_down(u32 usec)
{
    if (usec == -2) {
        wdt_close();
    }

    //睡眠前做预擦除动作
    sysmem_pre_erase_api();
    vPortSuppressTicksAndSleep(usec);

    wdt_init(WDT_8S);
}
#include "tick_timer_driver.h"
static u32 reserve;
static u32 reserve_2ms;
void sleep_time_compensate_callback(void *priv, u32 usec)
{
    local_irq_disable();

    u32 sys_jiffies = maskrom_get_jiffies();
    u32 tmp_usec = usec + reserve;
    sys_jiffies = sys_jiffies + (tmp_usec / (1000 * 10));
    reserve = (tmp_usec % (1000 * 10)); //保留不足10ms的余数
    maskrom_set_jiffies(sys_jiffies);

    /* 补偿jiffies_2ms */
    sys_jiffies = maskrom_get_jiffies_2ms();
    tmp_usec = usec + reserve_2ms;
    sys_jiffies = sys_jiffies + (tmp_usec / (1000 * 2));
    reserve_2ms = (tmp_usec % (1000 * 2)); //保留不足2ms的余数
    maskrom_set_jiffies_2ms(sys_jiffies);

    local_irq_enable();

}

#define BD49_VLVD_BASE_VOL  1800
u16 get_lvd_voltage(void)
{
    return BD49_VLVD_BASE_VOL + 100 * ((P3_VLVD_CON >> 3) & 0x7);
}

bool is_port_edge_wkup_source(void)
{
    extern u8 is_wakeup_source(enum WAKEUP_REASON index);
    if (is_wakeup_source(PWR_WK_REASON_PORT_EDGE)) {
        /* 边沿唤醒 */
        return 1;
    } else {
        /* 非边沿唤醒 */
        return 0;
    }
}

void key_active_set(P33_IO_WKUP_EDGE edge);
struct _p33_io_edge_wakeup_config port0 = {
    .gpio = POWER_WAKEUP_IO,
    .pullup_down_enable = 1,
    .filter = PORT_WKUP_FILTER,
    .edge = FALLING_EDGE,
    .callback = key_active_set,

};

#if TESTBOX_UART_UPDATE_EN
void testbox_uart_active_set();
struct _p33_io_edge_wakeup_config port1 = {
    .gpio = IO_PORTA_00,
    .pullup_down_enable = 1,
    .filter = PORT_WKUP_FILTER,
    .edge = FALLING_EDGE,
    .callback = NULL,//testbox_uart_active_set,
};
#endif

void power_io_wakeup_init(void)
{
    p33_io_wakeup_port_init(&port0);
#if TESTBOX_UART_UPDATE_EN
    p33_io_wakeup_port_init(&port1);
#endif
}

void vir_soff_keep_time(void);

void do_platform_uninitcall()
{
    vir_soff_keep_time();
}

u8 power_soff_callback()
{
    //可以添加其他需要在软关机前完成的事情
    do_platform_uninitcall();

    return 0;
}


