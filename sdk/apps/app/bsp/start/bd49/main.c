/*********************************************************************************************
    *   Filename        : main.c

    *   Description     :

    *   Author          :

    *   Email           :

    *   Last modifiled  :

    *   Copyright:(c)JIELI  2011-2017  @ , All Rights Reserved.
*********************************************************************************************/

#pragma bss_seg(".main.data.bss")
#pragma data_seg(".main.data")
#pragma const_seg(".main.text.const")
#pragma code_seg(".main.text")
#pragma str_literal_override(".main.text.const")

#include "config.h"
#include "common.h"
/* #include "gpio.h" */
#include "clock.h"
#include "maskrom.h"
/* #include "asm/power/p33.h" */
#include "app_config.h"
#include "init.h"
/* #include "init_app.h" */
#include "app.h"
/* #include "msg.h" */
#include "device.h"
/* #include "flash_init.h" */
/* #include "asm/power_interface.h" */
/* #include "power_api.h" */
/* #include "asm/debug.h" */
#include "wdt.h"
#include "efuse.h"

/* #include "vm.h" */
#include "power_interface.h"
/* #include "power_api.h" */

#include "my_malloc.h"
#include "sys_timer.h"
#include "icache.h"

#define LOG_TAG_CONST       MAIN
#define LOG_TAG             "[main]"
#include "log.h"

#include "msg.h"





/* extern void test_soft(void); */
/* void test_apa(void);*/
/* void vfs_demo_sydfs(void); */
/* void boot_info_test(void); */
/* void my_malloc_init(void); */
/* void app(void); */
/* void decoder_func(void); */
/* void app_system_init(void) */
/* { */
/* devices_init(); */
/* flash_system_init(); */

/* } */

extern const u8 config_no_osc_enable;

#if ICACHE_RAM_TO_RAM_ENABLE
extern int cache_ram_addr[];
extern int cache_ram_begin[];
extern int cache_ram_size[];
#endif

AT(.common)
void change_icache(void)
{
#if ICACHE_RAM_TO_RAM_ENABLE
    IcuSetWayNum(4 - ICACHE_RAM_TO_RAM / 4096);
    IcuFlushinvAll();
    memcpy((void *)cache_ram_addr, (void *)cache_ram_begin, (unsigned long)cache_ram_size);
#endif
}

void reset2update()
{
    /* 详细请看下载器升级章节 */
    /* 用户调用nvram_set_boot_state()函数后执行common_update_before_jump_reset_handle()可以直接复位进入maskrom进行 串口升级 或 USB升级 */
#if 0
    /* 该demo通过识别到PA9低电平后，复位进入maskrom进行 串口升级 或 USB升级  */
    /* TIPS: */
    /* 芯片开机后几乎所有IO会被锁住，直到board_power_init()执行后IO才解锁能够正常使用 */
    /* 除了部分IO是在RAM中使用没有被锁可以正常使用，如：PA0,PA9,PA10,USBDP,USBDM */
    gpio_set_mode(IO_PORT_SPILT(IO_PORTA_09), PORT_INPUT_FLOATING); //设置PA9为输入
    if (!(gpio_read(IO_PORTA_09))) {
        extern void nvram_set_boot_state(UPGRADE_TYPE up_type);
        extern void chip_reset();
        nvram_set_boot_state(UPGRADE_UART_SOFT_KEY);        //进入rom的串口升级
        /* nvram_set_boot_state(UPGRADE_USB_SOFTKEY);       //进入rom的usb升级 */
        common_update_before_jump_reset_handle();       //复位
    }
#endif

}

__attribute__((noreturn))
void c_main(int cfg_addr)
{
    JL_UART0->CON0 = 0;     //关闭rom串口升级
    change_icache();

    log_init(TCFG_UART_BAUDRATE);

    log_info("1 hello-word bd49\n");

    struct maskrom_argv mask_argv = {0};
    mask_argv.pchar = (void *)putchar;
    mask_argv.exp_hook = (void *)exception_analyze;
    mask_argv.local_irq_enable = NULL;
    mask_argv.local_irq_disable = NULL;
    mask_argv.flt = NULL;

    HWI_Install(1, (u32)exception_irq_handler, 7) ;
    mask_init(&mask_argv);
    emu_init();
    wdt_init(WDT_8S);
    efuse_init();
    /* clk_out0(6,CLK_OUT_PLL_96M); */
    /* clk_out0(6,CLK_OUT_STD_12M); */
    /* clk_out2(6,CLK_OUT2_STD_48M,1); */
    /* clk_out2(6,CLK_OUT2_SYSPLL_D2P0,1); */
    clk_voltage_init(CLOCK_MODE_ADAPTIVE, DVDD_VOL_129V);
    if (config_no_osc_enable) {
        clk_early_init(PLL_REF_LRC, 200000, 480000000);
    } else {
        clk_early_init(PLL_REF_XOSC_DIFF, 24000000, 480000000);
    }
    clock_dump();
    clk_set("sys", TCFG_SYS_PLL_CLK);


    /* while(1); */

    //test: pmu_oe
    JL_WLA->WLA_CON10 |= BIT(15);
    //reinit uart io
    /*JL_OMAP->PA5_OUT = 0;*/
    /*JL_PORTA->DIR &= ~BIT(6);*/
    /*JL_OMAP->PA6_OUT = FO_UART1_TX;*/

    //==============TODO: reinit uart
    //port_init();

    log_info("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    log_info("        bd49 setup %s-%s", __DATE__, __TIME__);
    log_info("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    efuse_dump();

    my_malloc_init();

#if SYS_TIMER_EN
    sys_timer_init();
#endif

    power_early_flowing();
    board_power_init();
    power_io_wakeup_init();


    /* test_soft(); */
    /* test_apa(); */
    /* boot_info_test(); */
    /* my_malloc_init(); */
    /* vfs_demo_sydfs(); */
    system_init();

    power_later_init();

    //fat测试
    /* void fat_demo(void); */
    /* fat_demo(); */

    //设备升级测试
    /* y_printf("\n >>>[test]:func = %s,line= %d\n",__FUNCTION__, __LINE__); */
    /* delay_10ms(100); */
    /* u16 dev_update_check(char *logo); */
    /* int err = dev_update_check("sd0"); */
    /* y_printf(">>>[test]:check err??????????? err = 0x%x\n", err); */


    app();
    //audio_adc_demo();
    /* decoder_func(); */
    u32 i = 0;
    while (1) {
        if (i > 0xfffff) {
            log_char('.');
            i = 0;
        }
        i++;
    }
#if 0
    wdt_close();

    mask_init_for_app();
    irq_init();
    local_irq_disable();
    local_irq_enable();

    //上电初始化所有IO
    port_init();

    log_init(TCFG_UART_BAUDRATE);

    pll_sel(TCFG_PLL_SEL, TCFG_PLL_DIV, TCFG_HSB_DIV);

    dump_clock_info();

    debug_init();

    wdt_init(WDT_8S);

    P3_PINR_CON &= ~BIT(0); //关闭长按复位

    /* gpio_clk_out(IO_PORTC_00, CLK_OUT_HSB); */

    log_info("time & date %s %s \n  OTP c_main\n", __TIME__, __DATE__);

    power_reset_source_dump();
    power_wakeup_reason_dump();
    sys_power_init();

    system_init();
    app();
    while (1) {
        wdt_clear();
    }
#endif
}


