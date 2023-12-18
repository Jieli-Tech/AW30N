#include "hw_timer.h"

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[hw_tmr_cap]"
#include "log.h"

void hw_timer_capture_init(JL_TIMER_TypeDef *JL_TIMERx, u32 gpio)
{
    printf("func:%s, line:%d\n", __func__, __LINE__);
    int num = get_hw_timerx_num(JL_TIMERx);
    /* gpio_ich_sel_input_signal(gpio, INPUT_CH_SIGNAL_TIMER0_CAPTURE + num, INPUT_CH_TYPE_GP_ICH); */
    gpio_set_function(IO_PORT_SPILT(gpio), PORT_FUNC_TIMER0_CAPTURE + num);
    JL_TIMERx->CON = TIMER_PCLR;  //清PND,清寄存器
    JL_TIMERx->CNT = 0; //清除计数值
    JL_TIMERx->PRD = 0; //清除计数值
    JL_TIMERx->CON = (TIMER_CESL_IO | TIMER_SSEL_STD_24M | TIMER_PSET_4);//I/O口输入，24M时钟源，4分频
    JL_TIMERx->CON |= TIMER_MODE_RISE;  //上升沿捕获模式启动
    /* JL_TIMERx->CON |= TIMER_DUAL_EDGE_EN;   //使能双边沿捕获模式 */
}

//demo
static JL_TIMER_TypeDef *TIMERx;
u32 num = 0, num_last = 0, time_us = 0;
___interrupt
static void hw_timer_capture_cb()
{
    if (TIMERx->CON & TIMER_PND) {   //有PND
        TIMERx->CON |= TIMER_PCLR;   //清PND

        num_last = num;
        num = TIMERx->PRD;
        if (num_last > num) {   //溢出a
            time_us = (num + 0xffff + 1 - num_last) / (24 / 4);
        } else {
            time_us = (num - num_last) / (24 / 4);  //24:时钟源     4:4分频
        }
        printf("num:%d, num_last:%d, time:%dus\n", num, num_last, time_us);
    }
}
void hw_timer_capture_init_test()
{
    TIMERx = JL_TIMER1;
    gpio_set_mode(IO_PORT_SPILT(IO_PORTA_06), PORT_INPUT_PULLUP_10K);
    hw_timer_capture_init(TIMERx, IO_PORTA_06);  //PA6捕获
    request_irq(IRQ_TIME1_IDX, 1, hw_timer_capture_cb, 0);
}


//红外接收demo NEC格式
void hw_timer_nec_capture_init(JL_TIMER_TypeDef *JL_TIMERx, u32 gpio)
{
    printf("func:%s, line:%d\n", __func__, __LINE__);
    int num = get_hw_timerx_num(JL_TIMERx);
    /* gpio_ich_sel_input_signal(gpio, INPUT_CH_SIGNAL_TIMER0_CAPTURE + num, INPUT_CH_TYPE_GP_ICH); */
    gpio_set_function(IO_PORT_SPILT(gpio), PORT_FUNC_TIMER0_CAPTURE + num);
    JL_TIMERx->CON = TIMER_PCLR;  //清PND,清寄存器
    JL_TIMERx->CNT = 0; //清除计数值
    JL_TIMERx->PRD = 0; //清除计数值
    JL_TIMERx->CON = (TIMER_CESL_IRFLT | TIMER_SSEL_STD_24M | TIMER_PSET_32);//滤波输入，24M时钟源，32分频
    JL_TIMERx->CON |= TIMER_MODE_FALL;  //下降沿捕获模式启动
    /* JL_TIMERx->CON |= TIMER_DUAL_EDGE_EN;   //使能双边沿捕获模式 */
}
#define NEC_HEAD        13
#define NEC_LOGIC_0     1
#define NEC_LOGIC_1     2
#define NEC_LENGTH      32
static struct nec_data {
    u16 user;   //用户代码，16位
    u8 data;    //数据
    u8 _data;   //数据取反
} nec_data_test;
static u32 cnt = 0, cnt_old = 0, time_ms = 0, offset = 32, rx_data = 0;
___interrupt
static void hw_timer_nec_capture_cb()
{
    if (TIMERx->CON & TIMER_PND) {   //有PND
        TIMERx->CON |= TIMER_PCLR;   //清PND

        cnt_old = cnt;
        cnt = TIMERx->PRD;
        if (cnt_old > cnt) {   //溢出
            time_ms = (cnt + 0xffff + 1 - cnt_old) / (24000 / 32);
        } else {
            time_ms = (cnt - cnt_old)  / (24000 / 32);  //24:时钟源     4:4分频
        }
        /* printf("cnt:%d, cnt_old:%d, time:%dms\n", cnt, cnt_old, time_ms); */

        if (time_ms == NEC_HEAD) {
            offset = 0;
            rx_data = 0;
            printf("rx head \n");
        }

        if (offset != NEC_LENGTH) {
            if (time_ms == NEC_LOGIC_1) {
                rx_data |= (1 << offset);
                offset++;
            } else if (time_ms == NEC_LOGIC_0) {
                rx_data |= (0 << offset);
                offset++;
            } else if (time_ms == NEC_HEAD) {

            } else {
                printf("error code!!!\n");
                offset = 32;
                rx_data = 0;
            }
            if (offset == NEC_LENGTH) {
                nec_data_test.user = (u16)(rx_data & 0xffff);
                nec_data_test.data = (u8)((rx_data >> 16) & 0xff);
                nec_data_test._data = (u8)(rx_data >> 24);
                printf("rx_data %x\n", rx_data);
                printf("user:%x, data:%x, _data:%x\n", nec_data_test.user, nec_data_test.data, nec_data_test._data);
            }
        }
    }
}

int hw_timer_capture_filter(u32 ns)
{

    JL_IR->RFLT_CON = 0; //清0
    JL_IR->RFLT_CON |= (IRFLT_TSRC_STD_12M | IRFLT_PSEL_1024); //设置IRLFT时钟源和分频
    JL_IR->RFLT_CON |= (IRFLT_EN);  //使能
    return ns;
}

void hw_timer_nec_capture_init_test()
{
    TIMERx = JL_TIMER1;
    gpio_set_mode(IO_PORT_SPILT(IO_PORTA_07), PORT_INPUT_PULLDOWN_10K);

    hw_timer_capture_filter(1 / 38000 * 1000000000);

    hw_timer_nec_capture_init(TIMERx, IO_PORTA_07);  //PA6捕获
    request_irq(IRQ_TIME1_IDX, 1, hw_timer_nec_capture_cb, 0);
}
