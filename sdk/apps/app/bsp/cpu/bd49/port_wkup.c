#pragma bss_seg(".port_wkup.data.bss")
#pragma data_seg(".port_wkup.data")
#pragma const_seg(".port_wkup.text.const")
#pragma code_seg(".port_wkup.text")
#pragma str_literal_override(".port_wkup.text.const")

#include "typedef.h"
#include "hwi.h"
#include "gpio.h"
#include "port_wkup.h"

#define LOG_TAG_CONST       WKUP
#define LOG_TAG             "[PORT_WKUP]"
#include "log.h"

/**
 * 注意：JL_WAKEUP 区别于PMU管理的唤醒。可理解为一个独立的模块使用。但在低功耗的情况下，中断无效。
 */

//bd49
/* IO唤醒源: 25个唤醒源:
 *    事件0~24: PA0-PA15,PB0-PB6,USB_DP,USB_DM
 * */


static void (*port_wkup_irq_cbfun[PORT_WKUP_SRC_NUM_MAX])(u32 port, u32 pin, u32 edge) = {NULL};
/**
 * @brief 引脚中断函数
 */
___interrupt
void port_wkup_irq(void)
{
    u8 wakeup_index = 0;
    u8 port = 0;
    /* printf("png:0x%x\n", JL_WAKEUP->CON3); */
    for (; wakeup_index < PORT_WKUP_SRC_NUM_MAX; wakeup_index++) {
        if ((JL_WAKEUP->CON0 & BIT(wakeup_index)) && (JL_WAKEUP->CON3 & BIT(wakeup_index))) {
            JL_WAKEUP->CON2 |= BIT(wakeup_index);
            if (port_wkup_irq_cbfun[wakeup_index]) {
                /* printf(">"); */
                port = wakeup_index;
                if (wakeup_index >= 23) {
                    port = wakeup_index - 23 + IO_PORT_DP;
                }
                port_wkup_irq_cbfun[wakeup_index](port / IO_GROUP_NUM, port % IO_GROUP_NUM, (!!(JL_WAKEUP->CON1 & BIT(wakeup_index))) + 1);
            }
        }
    }
}


/*
 * @brief 使能IO口[唤醒/外部中断]
 * @parm port 端口 such as:IO_PORTA_00
 * @parm edge 检测边缘，1 下降沿，0 上升沿
 * @parm cbfun 中断回调函数
 * @return 0 成功，< 0 失败
 */
int port_wkup_interrupt_init(u32 port, u32 edge, void (*cbfun)(u32 port, u32 pin, u32 edge))
{
    u8 wkup_s = port;
    if (port >= IO_PORT_MAX) {
        log_error("%s parameter:port error!", __func__);
        return -1;
    }
    if (port >= IO_PORT_DP) {
        wkup_s = port - IO_PORT_DP + 23;
        /* } else if (port >= IO_PORTB_00) { */
        /*     wkup_s = port % 16 + 16; */
    }
    if (JL_WAKEUP->CON0 & BIT(wkup_s)) {
        log_error("PORT WKUP(%d,%d) has been used\n", wkup_s, port);
        return -1;
    }
    JL_WAKEUP->CON2 = 0xffffffff;  //clear pending
    if (edge) {
        JL_WAKEUP->CON1 |= BIT(wkup_s);  //detect falling edge
        gpio_set_mode(IO_PORT_SPILT(port), PORT_INPUT_PULLUP_10K);
    } else {
        JL_WAKEUP->CON1 &= ~BIT(wkup_s);  //detect rising edge
        gpio_set_mode(IO_PORT_SPILT(port), PORT_INPUT_PULLDOWN_10K);
    }

    if (cbfun) {
        port_wkup_irq_cbfun[wkup_s] = cbfun;
    }
    request_irq(IRQ_PORT_IDX, PORT_WKUP_IRQ_PRIORITY, port_wkup_irq, 0);//中断优先级3
    JL_WAKEUP->CON2 |= BIT(wkup_s);  //clear pending
    JL_WAKEUP->CON0 |= BIT(wkup_s);  //wakeup enable
    log_info("wkup_s:%d, port:%d [en:%x,png:%x](init ok)\n", wkup_s, port, JL_WAKEUP->CON0, JL_WAKEUP->CON3);
    return 0;
}

/*
 * @brief 失能IO口[唤醒/外部中断]
 * @parm port 端口 such as:IO_PORTA_00
 * @return null
 */
int port_wkup_interrupt_deinit(u32 port)
{
    u8 wkup_s = port;
    if (port >= IO_PORT_MAX) {
        log_error("%s parameter:port error!", __func__);
        return -1;
    }
    if (port >= IO_PORT_DP) {
        wkup_s = port - IO_PORT_DP + 23;
        /* } else if (port >= IO_PORTB_00) { */
        /*     wkup_s = port % 16 + 7; */
    }

    if (JL_WAKEUP->CON0 & BIT(wkup_s)) {
        JL_WAKEUP->CON0 &= ~BIT(wkup_s);
        JL_WAKEUP->CON2 |= BIT(wkup_s);
        port_wkup_irq_cbfun[wkup_s] = NULL;
        log_info("wkup_s:%d, port:%d (close)\n", wkup_s, port);
        //有ich直接return
        gpio_set_mode(IO_PORT_SPILT(port), PORT_HIGHZ);
    } else {
        log_error("port wkup source:%d has been closed!", wkup_s);
        return -2;
    }
    return 0;
}


/*******************************************************/
void port_irq_change_callback(u32 port, void (*cbfun)(u32, u32, u32))
{
    if (port >= IO_PORT_MAX) {
        log_error("%s parameter:port error!", __func__);
        return;
    }
    u8 wkup_s = port;
    if (port >= IO_PORT_DP) {
        wkup_s = port - IO_PORT_DP + 23;
        /* } else if (port >= IO_PORTB_00) { */
        /*     wkup_s = port % 16 + 7; */
    }

    port_wkup_irq_cbfun[wkup_s] = cbfun;
}
void port_irq_change_en_state(u32 port, u8 wkup_en)//wkup_en:1:en,0:disable
{
    if (port >= IO_PORT_MAX) {
        log_error("%s parameter:port error!", __func__);
        return;
    }
    u8 wkup_s = port;
    if (port >= IO_PORT_DP) {
        wkup_s = port - IO_PORT_DP + 23;
        /* } else if (port >= IO_PORTB_00) { */
        /*     wkup_s = port % 16 + 7; */
    }

    if (wkup_en) {
        JL_WAKEUP->CON2 |= BIT(wkup_s);//清一次pnd
        JL_WAKEUP->CON0 |= BIT(wkup_s);//引脚中断使能
        JL_WAKEUP->CON2 |= BIT(wkup_s);//清一次pnd
    } else {
        JL_WAKEUP->CON0 &= ~ BIT(wkup_s); //引脚中断使能
        JL_WAKEUP->CON2 |= BIT(wkup_s);//清一次pnd
    }
    /* log_info("change_en_state:port:%d,wkup_s:%d", port, wkup_s); */
}

//单
void port_irq_change_edge_state(u32 port, u32 edge)
{
    if (port >= IO_PORT_MAX) {
        log_error("%s parameter:port error!", __func__);
        return;
    }
    u8 wkup_s = port;
    if (port >= IO_PORT_DP) {
        wkup_s = port - IO_PORT_DP + 23;
        /* } else if (port >= IO_PORTB_00) { */
        /*     wkup_s = port % 16 + 7; */
    }

    if (edge != (!!(JL_WAKEUP->CON1 & BIT(wkup_s)))) {
        if (edge) {
            JL_WAKEUP->CON1 |= BIT(wkup_s);
        } else {
            JL_WAKEUP->CON1 &= ~BIT(wkup_s);//0:上升沿
        }
    }
    /* log_info("change_edge_state:port:%d,wkup_s:%d", port, wkup_s); */
}
u8 port_irq_get_edge_state(u32 port)
{
    if (port >= IO_PORT_MAX) {
        log_error("%s parameter:port error!", __func__);
        return -1;
    }
    u8 wkup_s = port;
    if (port >= IO_PORT_DP) {
        wkup_s = port - IO_PORT_DP + 23;
        /* } else if (port >= IO_PORTB_00) { */
        /*     wkup_s = port % 16 + 7; */
    }

    u8 edge_res = 0;
    if (JL_WAKEUP->CON1 & BIT(wkup_s)) { //fall
        edge_res |= 0x02;
    } else { //rise
        edge_res |= 0x01;
    }
    /* log_info("get_edge_state:%d,wkup_s:%d", port, i); */
    return edge_res;
}

/*********************************************************************************************************
 * ******************************           使用举例如下           ***************************************
 * ******************************************************************************************************/
#if 0
#define TEST_IO_P JL_PORTB
#define TEST_IO_N 6
void _pa0(u32 port, u32 edge)
{
    TEST_IO_P->OUT ^= BIT(TEST_IO_N);
    printf("%s\n", __func__);
}
void _pa1(u32 port, u32 edge)
{
    TEST_IO_P->OUT ^= BIT(TEST_IO_N);
    printf("%s\n", __func__);
}

void _pb6(u32 port, u32 edge)
{
    TEST_IO_P->OUT ^= BIT(TEST_IO_N);
    printf("%s\n", __func__);
}

void _dp(u32 port, u32 edge)
{
    TEST_IO_P->OUT ^= BIT(TEST_IO_N);
    printf("%s\n", __func__);
}
#define _EDGE_1  0 //0:rise 1:fall
#define _EDGE_2  1
void mdelay(u32 ms);
void wdt_clear();
void port_wkup_test()
{
    printf("-------------------port wkup isr---------------------------\n");
    printf("wkupcon0:%x,con1:%x,con3:%x\n", JL_WAKEUP->CON0, JL_WAKEUP->CON1, JL_WAKEUP->CON3);
    TEST_IO_P->OUT &= ~BIT(TEST_IO_N);
    TEST_IO_P->DIR &= ~BIT(TEST_IO_N);
    port_wkup_interrupt_init(IO_PORTA_00, _EDGE_1, _pa0);//上升沿触发
    port_wkup_interrupt_init(IO_PORTA_01, _EDGE_1, _pa1);//下升沿触发
    port_wkup_interrupt_init(IO_PORTB_06, _EDGE_2, _pb6);//下升沿触发

    port_wkup_interrupt_init(IO_PORT_DP, _EDGE_2, _dp);//下升沿触发
    /* port_ich_wkup_enable(IO_PORTA_07, _EDGE_1, _ich0);//下升沿触发 */
    /* port_ich_wkup_enable(IO_PORTA_08, _EDGE_1, _ich1);//下升沿触发 */
    port_wkup_interrupt_deinit(IO_PORTB_06);
    /* port_ich_wkup_disable(IO_PORTB_03); */
    while (1) {
        printf("-");
        mdelay(100);
        wdt_clear();
    }
}
#endif
