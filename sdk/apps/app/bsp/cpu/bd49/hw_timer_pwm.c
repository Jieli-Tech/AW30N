#include "hw_timer.h"

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[hw_tmr_pwm]"
#include "log.h"

void hw_timer_pwm_init(JL_TIMER_TypeDef *JL_TIMERx, u32 gpio, u32 freq, u32 duty)
{
    printf("func:%s, line:%d\n", __func__, __LINE__);
    int num = get_hw_timerx_num(JL_TIMERx);
    /* gpio_mux_out(gpio,  GPIO_OUT_TIMER0_PWM + num * 0x100 + num); */
    /* gpio_och_sel_output_signal(gpio, OUTPUT_CH_SIGNAL_TIMER0_PWM + num); */
    gpio_set_function(IO_PORT_SPILT(gpio), PORT_FUNC_TIMER0_PWM + num);
    JL_TIMERx->CON = TIMER_PCLR;  //清PND,清寄存器
    JL_TIMERx->CNT = 0; //清除计数值
    JL_TIMERx->PRD = 24000000 / (4 * freq);  //设置计数值
    JL_TIMERx->PWM = (JL_TIMERx->PRD * duty) / 10000;  //设置占空比
    JL_TIMERx->CON = (TIMER_SSEL_STD_24M | TIMER_PSET_4); //24M时钟源，4分频
    JL_TIMERx->CON |= (TIMER_MODE_TIME | TIMER_PWM_EN); //计时模式启动，输出PWM
}

void hw_timer_pwm_set_duty(JL_TIMER_TypeDef *JL_TIMERx, u32 duty)
{
    JL_TIMERx->PWM = (JL_TIMERx->PRD * duty) / 10000;  //设置占空比
}

//demo
static JL_TIMER_TypeDef *TIMERx;
void hw_timer_pwm_init_test()
{
    TIMERx = JL_TIMER2;
    gpio_set_mode(IO_PORT_SPILT(IO_PORTA_07), PORT_OUTPUT_LOW);
    hw_timer_pwm_init(TIMERx, IO_PORTA_07, 1000, 5000); //A7,1K,50%
    hw_timer_pwm_set_duty(TIMERx, 5000);
}

#if 0
//IO口模拟红外发射的波形
static void udma_test(JL_TIMER_TypeDef *JL_TIMERx, u32 gpio);
void hw_timer_udma_test(JL_TIMER_TypeDef *JL_TIMERx, u32 gpio_pwm, u32 gpio_ir)
{
    hw_timer_pwm_init(JL_TIMERx, gpio_pwm, 1000000 / 560, 5000); //A7,1K,50%
    udma_test(JL_TIMERx, gpio_ir);
}

static u32 data = 0x11eedd22;   //32位待发送的数据
static u8 buf[24 + 4 * 32], len = 0;
static void make_buf(u32 gpio)
{
    u8 io_num = BIT(gpio % 16);

    memset(&buf[len], 0, 16);
    len += 16;
    memset(&buf[len], io_num, 8);
    len += 8;
    for (u8 i = 0; i < 32; i++) {
        if (data & 0x01) {
            buf[len] = 0;
            len += 1;
            memset(&buf[len], io_num, 3);
            len += 3;
        } else {
            buf[len] = 0;
            len += 1;
            buf[len] = io_num;
            len += 1;
        }
        data >>= 1;
    }
    buf[len] = 0;   //结尾拉低
    len += 1;
    printf_buf(buf, len);
    printf("len = %d\n", len);
}
static void udma_test(JL_TIMER_TypeDef *JL_TIMERx, u32 gpio)
{
    printf("Youbaibai udma test\n");
    make_buf(gpio);
    gpio_set_direction(gpio, 0);
    gpio_write(gpio, 1);

    dma_cfg cfg;
    cfg.src_addr = (u32)buf;
    cfg.dest_addr = (u32)&JL_PORTA->OUT;
    cfg.buffersize = len;
    cfg.ext_mode = DMA_EXT_MODE_0_EXPAND;
    cfg.src_inc = DMA_INC_ENABLE;
    cfg.dest_inc = DMA_INC_DISABLE;
    cfg.src_data_size = DMA_DATA_SIZE_BYTE;
    cfg.dest_data_size = DMA_DATA_SIZE_BYTE;
    cfg.mode = DMA_MODE_NORMAL;
    cfg.priority = DMA_PRIORITY_VERY_HIGH;
    cfg.M2M = DMA_M2M_DISABLE;
    cfg.cycle_time = 0;
    cfg.isr_cb = NULL;
    cfg.gen_req_num = 0;
    cfg.gen_pol = DMA_GEN_POL_RISING_EDGE;
    cfg.gen_sel = DMA_GEN_BIT_TMR0_PWM + get_hw_timerx_num(JL_TIMERx);

    dma_init(DMA0, &cfg);
    /* dma_int_config(DMA0, DMA_INT_CC, 1); */
    dma_int_config(DMA0, DMA_INT_TC, 1);
    /* dma_int_config(DMA0, DMA_INT_TH, 1); */
    dma_cmd(DMA0, DMA_CMD_GEN_ENABLE);
    dma_cmd(DMA0, DMA_CMD_ENABLE);

}
#endif
