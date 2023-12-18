#include "hw_timer.h"

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[hw_tmr]"
#include "log.h"

void hw_timer_init(JL_TIMER_TypeDef *JL_TIMERx, u32 msec)
{
    printf("func:%s, line:%d\n", __func__, __LINE__);
    JL_TIMERx->CON = TIMER_PCLR;  //清PND, 清寄存器
    JL_TIMERx->CNT = 0; //清除计数器
    JL_TIMERx->PRD = 24000000 / 1024 * msec / 1000;  //设置计数值，最大计时时间 = 1/(时钟源/分频系数)*65535，单位:s
    JL_TIMERx->CON = (TIMER_SSEL_STD_24M | TIMER_PSET_1024); //24M时钟源，1024分频
    JL_TIMERx->CON |= (TIMER_MODE_TIME); //计时模式启动
}

//demo
static JL_TIMER_TypeDef *TIMERx;
___interrupt
static void hw_timer_time_cb()
{
    if (TIMERx->CON & TIMER_PND) {   //有PND
        TIMERx->CON |= TIMER_PCLR;   //清PND
        JL_PORTA->OUT ^= BIT(7);        //翻转I/O
    }
}
void hw_timer_init_test()
{
    TIMERx = JL_TIMER0;
    hw_timer_init(TIMERx, 100); //定时器初始化，100ms
    request_irq(IRQ_TIME0_IDX, 1, hw_timer_time_cb, 0);    //设置定时器中断(中断号，优先级，回调函数，cpu核)

    gpio_set_mode(IO_PORT_SPILT(IO_PORTA_07), PORT_OUTPUT_LOW);
    /* gpio_set_direction(IO_PORTA_07, 0);  //I/O设为输出 */
    /* gpio_set_pull_up(IO_PORTA_07, 0); */
    /* gpio_set_pull_down(IO_PORTA_07, 0); */
}
