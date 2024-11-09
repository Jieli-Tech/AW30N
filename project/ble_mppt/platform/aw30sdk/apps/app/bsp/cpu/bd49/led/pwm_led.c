#include "pwm_led.h"
#include "gpio.h"
#include "asm/power_interface.h"
#include "app_config.h"


#if 0
#define log_debug   printf
#else
#define log_debug(...)
#endif



enum led_clk_div {
    led_clk_div1    = 0b0000,
    led_clk_div4    = 0b0001,
    led_clk_div16   = 0b0010,
    led_clk_div64   = 0b0011,
    led_clk_div2    = 0b0100,
    led_clk_div8    = 0b0101,
    led_clk_div32   = 0b0110,
    led_clk_div128  = 0b0111,
    led_clk_div256  = 0b1000,
};

const static u8 led_clk_div_table[9] = {
    led_clk_div1,
    led_clk_div2,
    led_clk_div4,
    led_clk_div8,
    led_clk_div16,
    led_clk_div32,
    led_clk_div64,
    led_clk_div128,
    led_clk_div256,
};

static struct pwm_led_platform_data pled_pdata;
#define __this      (&pled_pdata)

static u8 pwm_led_data_init = 0;
static u8 pwm_led_ctl_cnt;
static u8 soft_alternate_out;
static u32 pwm_led_ctl_unit;
extern u32 __get_lrc_hz();
#define PWM_LED_CLK (__get_lrc_hz() / 10)



/*
 * @brief  PWM_LED模块中断服务函数
*/
___interrupt
static void pwm_led_isr(void)
{
    JL_PLED->CON3 |= BIT(6);
    pwm_led_ctl_cnt ++;
    if ((__this->ctl_cycle_num) && (pwm_led_ctl_cnt >= __this->ctl_cycle_num))  {
        pwm_led_hw_close();
        if (__this->cbfunc) {
            __this->cbfunc(pwm_led_ctl_cnt);
        }
    } else if (soft_alternate_out) {
        if ((pwm_led_ctl_cnt % 2)) {
            gpio_set_mode(IO_PORT_SPILT(__this->port0), PORT_HIGHZ);
            gpio_set_function(IO_PORT_SPILT(__this->port1), PORT_FUNC_PWM_LED);
        } else {
            gpio_set_mode(IO_PORT_SPILT(__this->port1), PORT_HIGHZ);
            gpio_set_function(IO_PORT_SPILT(__this->port0), PORT_FUNC_PWM_LED);
        }
    }
}

static void pwm_led_set_h_pwm_duty(u32 pwm_prd, u32 pwm_duty)
{
    u32 h_pwm_duty_prd = pwm_prd * pwm_duty / 100;
    if (h_pwm_duty_prd) {
        h_pwm_duty_prd += 1;
        h_pwm_duty_prd = h_pwm_duty_prd > pwm_prd ? pwm_prd : h_pwm_duty_prd;
    }
    JL_PLED->BRI_DUTY0H = ((h_pwm_duty_prd >> 8) & 0xff);
    JL_PLED->BRI_DUTY0L = ((h_pwm_duty_prd >> 0) & 0xff);
}

static void pwm_led_set_l_pwm_duty(u32 pwm_prd, u32 pwm_duty)
{
    u32 l_pwm_duty_prd = pwm_prd * pwm_duty / 100;
    if (l_pwm_duty_prd) {
        l_pwm_duty_prd += 1;
        l_pwm_duty_prd = l_pwm_duty_prd > pwm_prd ? pwm_prd : l_pwm_duty_prd;
    }
    JL_PLED->BRI_DUTY1H = ((l_pwm_duty_prd >> 8) & 0xff);
    JL_PLED->BRI_DUTY1L = ((l_pwm_duty_prd >> 0) & 0xff);
}

static void pwm_led_set_pwm_out_once_time(u32 t_cnt3)
{
    u32 t_cnt0 = 0;
    u32 t_cnt1 = 0;
    u32 t_cnt2 = 0;
    u32 factor = 1000;
    u32 ctl_cycle = __this->ctl_cycle * factor;
    u32 pwm_out_time = (__this->out_once.pwm_out_time & 0x7fffffff) * factor;
    u32 cnt = pwm_out_time * t_cnt3 / ctl_cycle;
    if (cnt) {
        if (!(__this->out_once.pwm_out_time & BIT(31))) {//out defer
            t_cnt0 = 0;
            t_cnt1 = t_cnt3 - cnt;
            t_cnt1 = t_cnt1 < 1 ? 1 : t_cnt1;
            t_cnt2 = t_cnt3;
        } else {
            t_cnt0 = 0;
            t_cnt1 = 1;
            t_cnt2 = t_cnt1 + cnt;
            t_cnt2 = t_cnt2 > t_cnt3 ? t_cnt3 : t_cnt2;
        }
    } else {
        t_cnt3 = 0;
    }
    JL_PLED->DUTY0 = t_cnt0;
    JL_PLED->DUTY1 = t_cnt1;
    JL_PLED->DUTY2 = t_cnt2;
    JL_PLED->DUTY3 = t_cnt3;
    JL_PLED->CON1 |= (!!t_cnt0) * BIT(4);//PWM_DUTY0_EN
    JL_PLED->CON1 |= (!!t_cnt1) * BIT(5);//PWM_DUTY1_EN
    JL_PLED->CON1 |= (!!t_cnt2) * BIT(6);//PWM_DUTY2_EN
    JL_PLED->CON1 |= (!!t_cnt3) * BIT(7);//PWM_DUTY3_EN
    //JL_PLED->CON1 |= BIT(3);
}

static void pwm_led_set_pwm_out_twice_time(u32 t_cnt3)
{
    u32 cnt0, t_cnt0 = 0;
    u32 cnt1, t_cnt1 = 0;
    u32 cnt2, t_cnt2 = 0;
    u32 factor = 1000;
    u32 ctl_cycle = __this->ctl_cycle * factor;
    u32 first_pwm_out_time = __this->out_twice.first_pwm_out_time * factor;
    u32 second_pwm_out_time = __this->out_twice.second_pwm_out_time * factor;
    u32 pwm_gap_time = __this->out_twice.pwm_gap_time * factor;
    u32 highz_time = ctl_cycle - (first_pwm_out_time + pwm_gap_time + second_pwm_out_time);
    if (highz_time == ctl_cycle) {
        t_cnt3 = 0;
    } else {
        cnt0 = highz_time          * t_cnt3 / ctl_cycle;
        cnt1 = first_pwm_out_time  * t_cnt3 / ctl_cycle;
        cnt2 = pwm_gap_time        * t_cnt3 / ctl_cycle;
        t_cnt0 = cnt0 < 1 ? 1 : cnt0;
        t_cnt1 = t_cnt0 + cnt1;
        t_cnt1 = (t_cnt1 < (t_cnt0 + 1)) ? (t_cnt0 + 1) : t_cnt1;
        t_cnt2 = t_cnt1 + cnt2;
        t_cnt2 = (t_cnt2 < (t_cnt1 + 1)) ? (t_cnt1 + 1) : t_cnt2;
    }
    JL_PLED->DUTY0 = t_cnt0;
    JL_PLED->DUTY1 = t_cnt1;
    JL_PLED->DUTY2 = t_cnt2;
    JL_PLED->DUTY3 = t_cnt3;
    JL_PLED->CON1 |= (!!t_cnt0) * BIT(4);//PWM_DUTY3_EN
    JL_PLED->CON1 |= (!!t_cnt1) * BIT(5);//PWM_DUTY2_EN
    JL_PLED->CON1 |= (!!t_cnt2) * BIT(6);//PWM_DUTY1_EN
    JL_PLED->CON1 |= (!!t_cnt3) * BIT(7);//PWM_DUTY0_EN
    //JL_PLED->CON1 |= BIT(3);
}

static void pwm_led_fixed_output_mode(void)
{
    JL_PLED->CON2 &= ~(0b1111 << 4);
    JL_PLED->CON2 |= ((__this->alternate_out & 0b111) << 4);

    u32 factor = 1000;
    u32 ctl_cycle = __this->ctl_cycle * factor;
    u32 ctl_num = 255;
    u32 ctl_unit = ctl_cycle / ctl_num;
    u32 div_idx = 0;
    u32 ctl_prd_div, div;
__get_ctl_div:
    div = 1 << div_idx;
    ctl_prd_div = PWM_LED_CLK * ctl_unit / (100000 * div);
    if (ctl_prd_div > 4096) {
        div_idx ++;
        goto __get_ctl_div;
    }
    log_debug("ctl_prd_div = %d\n", ctl_prd_div);
    if (ctl_prd_div) {
        JL_PLED->PRD_DIVL = (ctl_prd_div - 1) & 0xff;
        JL_PLED->CON3 |= ((ctl_prd_div - 1) >> 8) & 0xf;
    } else {
        JL_PLED->PRD_DIVL = 0;
    }

    log_debug("clk_div = %d\n", div);
    u8 div_reg_value = led_clk_div_table[div_idx];
    SFR(JL_PLED->CON0, 4, 4, div_reg_value);//时钟源分频

    u32 pwm_cycle = __this->pwm_cycle * 10;
    u32 pwm_prd = PWM_LED_CLK * pwm_cycle / (100000 * div);
    log_debug("pwm_prd = %d\n", pwm_prd);
    JL_PLED->BRI_PRDH = (pwm_prd >> 8) & 0b11;
    JL_PLED->BRI_PRDL = (pwm_prd >> 0) & 0xff;

    if ((__this->first_logic == 0) || (__this->alternate_out)) {
        pwm_led_set_h_pwm_duty(pwm_prd, __this->h_pwm_duty);
    }
    if ((__this->first_logic == 1) || (__this->alternate_out)) {
        pwm_led_set_l_pwm_duty(pwm_prd, __this->l_pwm_duty);
    }
    if (__this->out_mode == 0) {
        pwm_led_set_pwm_out_once_time(ctl_num);
    } else {
        pwm_led_set_pwm_out_twice_time(ctl_num);
    }
    pwm_led_ctl_unit = ctl_unit;
}

static void pwm_led_breathe_output_mode(void)
{
    JL_PLED->CON0 |= BIT(1);    //呼吸变化模式
    JL_PLED->CON2 &= ~(0b1111 << 4);
    JL_PLED->CON2 |= ((__this->alternate_out & 0b111) << 5);

    u32 factor = 1000;
    u32 ctl_cycle = __this->ctl_cycle * factor;
    u32 pwm_out_time = __this->out_breathe.pwm_out_time * factor;
    u32 pwm_duty_max_keep_time = __this->out_breathe.pwm_duty_max_keep_time * factor;
    if (pwm_duty_max_keep_time > (pwm_out_time / 2)) {
        pwm_duty_max_keep_time = pwm_out_time / 2;
    }
    u32 pwm_duty_change_time = (pwm_out_time - pwm_duty_max_keep_time) / 2;
    log_debug("ctl_cycle = %d", ctl_cycle);
    log_debug("pwm_out_time = %d", pwm_out_time);
    log_debug("pwm_duty_max_keep_time = %d", pwm_duty_max_keep_time);
    log_debug("pwm_duty_change_time = %d", pwm_duty_change_time);

    u32 h_pwm_duty = 0;
    u32 l_pwm_duty = 0;

    u32 div_idx = 0;
    u32 ctl_prd_div, div = 1;
    u32 ctl_num, ctl_unit;
    u32 pwm_prd;
    u32 pwm_cycle = __this->pwm_cycle * 10;
__get_ctl_div:
    pwm_prd = PWM_LED_CLK * pwm_cycle / (100000 * div);
    log_debug("pwm_prd = %d\n", pwm_prd);
    ctl_num = 0;
    if ((__this->first_logic == 0) || (__this->alternate_out)) {
        h_pwm_duty = pwm_prd * __this->h_pwm_duty / 100;
        ctl_num = ctl_num < h_pwm_duty ? h_pwm_duty : ctl_num;
    }
    if ((__this->first_logic == 1) || (__this->alternate_out)) {
        l_pwm_duty = pwm_prd * __this->l_pwm_duty / 100;
        ctl_num = ctl_num < l_pwm_duty ? l_pwm_duty : ctl_num;
    }
    if (ctl_num) {
        ctl_unit = pwm_duty_change_time / ctl_num;
    } else {
        ctl_unit = pwm_cycle;
    }
    div = 1 << div_idx;
    ctl_prd_div = PWM_LED_CLK * ctl_unit / (100000 * div);
    if (ctl_prd_div > 4096) {
        div_idx ++;
        goto __get_ctl_div;
    }
    log_debug("ctl_num = %d\n", ctl_num);
    log_debug("ctl_unit = %d\n", ctl_unit);
    log_debug("ctl_prd_div = %d\n", ctl_prd_div);
    if (ctl_prd_div) {
        JL_PLED->PRD_DIVL = (ctl_prd_div - 1) & 0xff;
        JL_PLED->CON3 |= ((ctl_prd_div - 1) >> 8) & 0xf;
    } else {
        JL_PLED->PRD_DIVL = 0;
    }

    log_debug("clk_div = %d\n", div);
    u32 div_reg_value = led_clk_div_table[div_idx];
    SFR(JL_PLED->CON0, 4, 4, div_reg_value);//时钟源分频
    JL_PLED->BRI_PRDH = (pwm_prd >> 8) & 0b11;
    JL_PLED->BRI_PRDL = (pwm_prd >> 0) & 0xff;

    if ((__this->first_logic == 0) || (__this->alternate_out)) {
        pwm_led_set_h_pwm_duty(pwm_prd, __this->h_pwm_duty);
        u32 h_pwm_duty_change_time = ctl_unit * h_pwm_duty * 2;
        u32 h_pwm_duty_max_keep_time = 0;
        if (pwm_out_time > h_pwm_duty_change_time) {
            h_pwm_duty_max_keep_time = pwm_out_time - h_pwm_duty_change_time;
        }
        log_debug("h_pwm_duty_change_time = %d", h_pwm_duty_change_time);
        log_debug("h_pwm_duty_max_keep_time = %d", h_pwm_duty_max_keep_time);
        u32 h_pwm_duty_max_keep_prd = h_pwm_duty_max_keep_time / ctl_unit;
        h_pwm_duty_max_keep_prd = h_pwm_duty_max_keep_prd > 255 ? 255 : h_pwm_duty_max_keep_prd;
        JL_PLED->DUTY1 = h_pwm_duty_max_keep_prd;
    }
    if ((__this->first_logic == 1) || (__this->alternate_out)) {
        pwm_led_set_l_pwm_duty(pwm_prd, __this->l_pwm_duty);
        u32 l_pwm_duty_change_time = ctl_unit * l_pwm_duty * 2;
        u32 l_pwm_duty_max_keep_time = 0;
        if (pwm_out_time > l_pwm_duty_change_time) {
            l_pwm_duty_max_keep_time = pwm_out_time - l_pwm_duty_change_time;
        }
        log_debug("l_pwm_duty_change_time = %d", l_pwm_duty_change_time);
        log_debug("l_pwm_duty_max_keep_time = %d", l_pwm_duty_max_keep_time);
        u32 l_pwm_duty_max_keep_prd = l_pwm_duty_max_keep_time / ctl_unit;
        l_pwm_duty_max_keep_prd = l_pwm_duty_max_keep_prd > 255 ? 255 : l_pwm_duty_max_keep_prd;
        JL_PLED->DUTY0 = l_pwm_duty_max_keep_prd;
    }

    u32 pwm_duty_0_keep_time = ctl_cycle - pwm_out_time;
    log_debug("pwm_duty_0_keep_time = %d", pwm_duty_0_keep_time);
    u32 pwm_duty_0_keep_prd = pwm_duty_0_keep_time / ctl_unit;
    JL_PLED->DUTY3 = ((pwm_duty_0_keep_prd >> 8) & 0xff);
    JL_PLED->DUTY2 = ((pwm_duty_0_keep_prd >> 0) & 0xff);

    JL_PLED->CON1 |=  BIT(7);//PWM_DUTY3_EN
    JL_PLED->CON1 |=  BIT(6);//PWM_DUTY2_EN
    JL_PLED->CON1 |=  BIT(5);//PWM_DUTY1_EN
    JL_PLED->CON1 |=  BIT(4);//PWM_DUTY0_EN

    pwm_led_ctl_unit = ctl_unit;
}


void pwm_led_io_mount(void)
{
    if (pwm_led_data_init == 0) {
        return ;
    }
    soft_alternate_out = 0;
    if ((__this->port0 < IO_PORT_MAX) && \
        (__this->port1 < IO_PORT_MAX) && \
        (__this->port0 != __this->port1) && \
        (__this->h_pwm_duty == __this->l_pwm_duty) && \
        (__this->alternate_out)) {
        soft_alternate_out = 1;
        __this->alternate_out = 0;
        JL_PLED->CON3 |= BIT(5);
        request_irq(IRQ_PWM_LED_IDX, 1, pwm_led_isr, 0);
        gpio_set_function(IO_PORT_SPILT(__this->port0), PORT_FUNC_PWM_LED);
        gpio_set_mode(IO_PORT_SPILT(__this->port1), PORT_HIGHZ);
    } else {
        if (__this->port0 < IO_PORT_MAX) {
            gpio_set_function(IO_PORT_SPILT(__this->port0), PORT_FUNC_PWM_LED);
        }
        if (__this->port1 < IO_PORT_MAX) {
            gpio_set_function(IO_PORT_SPILT(__this->port1), PORT_FUNC_PWM_LED);
        }
    }
}

void pwm_led_io_unmount(void)
{
    if (pwm_led_data_init == 0) {
        return ;
    }
    if (__this->port0 < IO_PORT_MAX) {
        gpio_set_mode(IO_PORT_SPILT(__this->port0), PORT_HIGHZ);
    }
    if (__this->port1 < IO_PORT_MAX) {
        gpio_set_mode(IO_PORT_SPILT(__this->port1), PORT_HIGHZ);
    }
}

void pwm_led_dump(void)
{
#if 1
    printf("port0 = %d\n", __this->port0);
    printf("port1 = %d\n", __this->port1);
    printf("first_logic = %d\n", __this->first_logic);
    printf("alternate_out = %d\n", __this->alternate_out);
    printf("pwm_cycle = %u\n", __this->pwm_cycle);
    printf("ctl_cycle = %u\n", __this->ctl_cycle);
    printf("h_pwm_duty = %d\n", __this->h_pwm_duty);
    printf("l_pwm_duty = %d\n", __this->l_pwm_duty);
    printf("mode = %d\n", __this->out_mode);
    printf("ctl_cycle_num = %d\n", __this->ctl_cycle_num);
    printf("cbfunc = %p\n", __this->cbfunc);
    switch (__this->out_mode) {
    case 0:
        printf("out_once.pwm_out_time = %u\n", __this->out_once.pwm_out_time);
        break;
    case 1:
        printf("out_twice.first_pwm_out_time = %u\n", __this->out_twice.first_pwm_out_time);
        printf("out_twice.second_pwm_out_time = %u\n", __this->out_twice.second_pwm_out_time);
        printf("out_twice.pwm_gap_time = %u\n", __this->out_twice.pwm_gap_time);
        break;
    case 2:
        printf("out_breathe.pwm_duty_max_keep_time = %u\n", __this->out_breathe.pwm_duty_max_keep_time);
        printf("out_breathe.pwm_out_time = %u\n", __this->out_breathe.pwm_out_time);
        break;
    }
#endif

#if 1
    printf("PLED_CLK = %d\n", __get_lrc_hz());
    printf("JL_PLED->CON0 = 0x%x\n", JL_PLED->CON0);
    printf("JL_PLED->CON1 = 0x%x\n", JL_PLED->CON1);
    printf("JL_PLED->CON2 = 0x%x\n", JL_PLED->CON2);
    printf("JL_PLED->CON3 = 0x%x\n", JL_PLED->CON3);
    printf("JL_PLED->CON4 = 0x%x\n", JL_PLED->CON4);
    printf("JL_PLED->BRI_PRDL = 0x%x\n", JL_PLED->BRI_PRDL);
    printf("JL_PLED->BRI_PRDH = 0x%x\n", JL_PLED->BRI_PRDH);
    printf("JL_PLED->BRI_DUTY0L = 0x%x\n", JL_PLED->BRI_DUTY0L);
    printf("JL_PLED->BRI_DUTY0H = 0x%x\n", JL_PLED->BRI_DUTY0H);
    printf("JL_PLED->BRI_DUTY1L = 0x%x\n", JL_PLED->BRI_DUTY1L);
    printf("JL_PLED->BRI_DUTY1H = 0x%x\n", JL_PLED->BRI_DUTY1H);
    printf("JL_PLED->PRD_DIV = 0x%x\n", JL_PLED->PRD_DIVL);
    printf("JL_PLED->DUTY0 = %d\n", JL_PLED->DUTY0);
    printf("JL_PLED->DUTY1 = %d\n", JL_PLED->DUTY1);
    printf("JL_PLED->DUTY2 = %d\n", JL_PLED->DUTY2);
    printf("JL_PLED->DUTY3 = %d\n", JL_PLED->DUTY3);
#endif
    printf("JL_PORTA->DIR = 0x%x\n", JL_PORTA->DIR);
    printf("JL_PORTA->DIE = 0x%x\n", JL_PORTA->DIE);
    printf("JL_PORTA->PU0 = 0x%x\n", JL_PORTA->PU0);
    printf("JL_PORTA->PU1 = 0x%x\n", JL_PORTA->PU1);
    printf("JL_PORTA->PD0 = 0x%x\n", JL_PORTA->PD0);
    printf("JL_PORTA->PD1 = 0x%x\n", JL_PORTA->PD1);
    printf("JL_PORTA->OUT = 0x%x\n", JL_PORTA->OUT);
}


/*
 * @brief  PWM_LED模块初始化函数
 * @arg pdata 初始化的参数结构体地址： struct pwm_led_platform_data *
 */
void pwm_led_hw_init(void *pdata)
{
    if (!pdata) {
        return;
    }
    memset((u8 *)JL_PLED, 0, sizeof(JL_PLED_TypeDef));
    memcpy((u8 *)__this, (u8 *)pdata, sizeof(struct pwm_led_platform_data));
    pwm_led_data_init = 1;
    pwm_led_ctl_cnt = 0;

    //初始化引脚
    pwm_led_io_mount();

    P33_CON_SET(P3_CLK_CON0, 0, 2, 3);  //LRC200K
    P33_CON_SET(P3_CLK_CON0, 3, 1, 0);  //LED_CLK_DIS
    JL_PLED->CON0 &= ~(0b11 << 2);      //PWM_LED选择LRD_200K做时钟源
    JL_PLED->CON0 &= ~(0b1111 << 4);    //时钟源不分频

    if (__this->first_logic == 0) {
        JL_PLED->CON1 &= ~BIT(2);
    } else {
        JL_PLED->CON1 |=  BIT(2);
    }
    JL_PLED->CON3 |= BIT(4);
    JL_PLED->CON3 |= BIT(6);

    if (__this->out_mode < 2) {
        pwm_led_fixed_output_mode();
    } else {
        pwm_led_breathe_output_mode();
    }
    if ((__this->cbfunc) || (__this->ctl_cycle_num)) {
        JL_PLED->CON3 |= BIT(5);
        request_irq(IRQ_PWM_LED_IDX, 1, pwm_led_isr, 0);
    }

    JL_PLED->CON0 |= BIT(0);

    /* pwm_led_dump(); */
}

/*
 * @brief  关闭pwm_led模块
*/
void pwm_led_hw_close(void)
{
    JL_PLED->CON0 &= ~BIT(0);
    pwm_led_io_unmount();
    pwm_led_data_init = 0;
}

u32 pwm_led_is_working(void)
{
    return (!!(JL_PLED->CON0 & BIT(0)));
}


static void pwm_led_get_next_dir_level(u32 cur_dir, u32 cur_level, u32 *dir, u32 *level)
{
    if (JL_PLED->CON0 & BIT(1)) {//呼吸变化模式
        *dir = !cur_dir;
        if (__this->alternate_out) {
            if (cur_dir) {
                *level = cur_level;
            } else {
                *level = !cur_level;
            }
        } else {
            *level = cur_level;
        }
    } else {
        *dir = cur_dir;
        if (__this->alternate_out) {
            *level = !cur_level;
        } else {
            *level = cur_level;
        }
    }
}

static u32 pwm_led_get_cur_status_cnt_max(u32 cur_dir, u32 cur_level)
{
    u32 keep_prd;
    u32 cnt_max;
    if (JL_PLED->CON0 & BIT(1)) {//呼吸变化模式
        u32 h_pwm_duty_prd = (JL_PLED->BRI_DUTY0H >> 8) | JL_PLED->BRI_DUTY0L;
        u32 l_pwm_duty_prd = (JL_PLED->BRI_DUTY1H >> 8) | JL_PLED->BRI_DUTY1L;
        if (cur_dir) {
            keep_prd = (JL_PLED->DUTY3 << 8) | JL_PLED->DUTY2;
            if (cur_level) {
                cnt_max = keep_prd + l_pwm_duty_prd;
            } else {
                cnt_max = keep_prd + h_pwm_duty_prd;
            }
        } else {
            if (cur_level) {
                keep_prd = JL_PLED->DUTY0;
                cnt_max = keep_prd + l_pwm_duty_prd;
            } else {
                keep_prd = JL_PLED->DUTY1;
                cnt_max = keep_prd + h_pwm_duty_prd;
            }
        }
    } else {
        cnt_max = JL_PLED->DUTY3;
    }
    return cnt_max;
}

/*
 * @brief  获取PWM_LED模块状态信息
 */
void pwm_led_get_sync_status(struct pwm_led_status_t *status)
{
    if (!status) {
        return;
    }
    status->cnt_max = 0;
    if ((JL_PLED->CON0 & BIT(0)) == 0) {
        return;
    }
    JL_PLED->CON4 |= BIT(4);
    while (!(JL_PLED->CON4 & BIT(5)));
    u32 cnt_rd = JL_PLED->CNT_RD;

    u32 next_dir = 0;
    u32 next_level = 0;
    status->dir   = !!(cnt_rd & BIT(17));
    status->level = !!(cnt_rd & BIT(16));
    status->cur_cnt  = cnt_rd & 0xffff;
    status->cnt_max = pwm_led_get_cur_status_cnt_max(status->dir, status->level);
    pwm_led_get_next_dir_level(status->dir, status->level, (u32 *)&next_dir, (u32 *)&next_level);
    status->next_cnt_max = pwm_led_get_cur_status_cnt_max(next_dir, next_level);
    status->cnt_unit = pwm_led_ctl_unit;
}

/*
 * @brief  设置PWM_LED暂停同步
 * @arg status 另一个样机的pwm_led的状态
 * @arg how_long_ago 另一个样机的pwm_led的状态是多久之前的, 单位us
 * @arg sync_time 如果为快的样机，则通过该变量获取同步暂停时间, 单位us
 * @return 0:成功  1:失败
 */
u32 pwm_led_set_sync(struct pwm_led_status_t *status, u32 how_long_ago, u32 *sync_time)
{
    if (!status) {
        return 1;
    }
    if (status->cnt_max == 0) {
        return 1;
    }
    if (status->next_cnt_max == 0) {
        return 1;
    }
    if ((status->cnt_unit == 0) || (pwm_led_ctl_unit == 0)) {
        return 1;
    }
    if ((JL_PLED->CON0 & BIT(0)) == 0) {
        return 1;
    }
    /* u32 dir   = status->dir; */
    /* u32 level = status->level; */
    u32 cnt_delta = how_long_ago / status->cnt_unit;
    u32 cnt = status->cur_cnt;
    u32 table_idle = 0;
    u32 cnt_max_table[2];
    cnt_max_table[0] = status->cnt_max;
    cnt_max_table[1] = status->next_cnt_max;
    u32 cnt_max = cnt_max_table[0];
    if (JL_PLED->CON0 & BIT(1)) {//呼吸变化模式
        for (u32 i = 0; i < cnt_delta; i ++) {
            cnt ++;
            if (cnt > cnt_max) {
                cnt = 0;
                table_idle = !table_idle;
                cnt_max = cnt_max_table[table_idle];
            }
        }
    } else {
        cnt += cnt_delta;
        cnt %= (cnt_max + 1);
    }
    u32 progress = 1000 * cnt / cnt_max;

    JL_PLED->CON4 |= BIT(4);
    while (!(JL_PLED->CON4 & BIT(5)));
    u32 cnt_rd = JL_PLED->CNT_RD;

    u32 cur_dir   = !!(cnt_rd & BIT(17));
    u32 cur_level = !!(cnt_rd & BIT(16));
    u32 cur_cnt  = cnt_rd & 0xffff;
    u32 cur_cnt_max  = pwm_led_get_cur_status_cnt_max(cur_dir, cur_level);
    u32 cur_progress = 1000 * cur_cnt / cur_cnt_max;
    u32 wait_time = 0;
    u32 diff;
    if (JL_PLED->CON0 & BIT(1)) {//呼吸变化模式
        if ((cnt_max_table[0] > cnt_max_table[1]) && (cnt_max == cnt_max_table[1]) && (cur_dir == 1)) {
            wait_time = ((cnt_max - cnt) + (cur_progress * cnt_max_table[0] / 1000)) * status->cnt_unit;
        }
        if ((cnt_max_table[0] < cnt_max_table[1]) && (cnt_max == cnt_max_table[0]) && (cur_dir == 1)) {
            wait_time = ((cnt_max - cnt) + (cur_progress * cnt_max_table[1] / 1000)) * status->cnt_unit;
        }
        if (cur_progress > progress) {
            diff = cur_progress - progress;
            wait_time = (diff * cnt_max / 1000) * status->cnt_unit;
        }
    } else {
        if ((cur_progress > progress) && ((cur_progress - progress) <= 500)) {
            diff = cur_progress - progress;
            wait_time = (diff * cnt_max / 1000) * status->cnt_unit;
        }
        if ((cur_progress < progress) && ((progress - cur_progress) >= 500)) {
            diff = 1000 - (progress - cur_progress);
            wait_time = (diff * cnt_max / 1000) * status->cnt_unit;
        }
    }

    *sync_time = 0;
    u32 sync = wait_time / pwm_led_ctl_unit;
    if (sync) {
        *sync_time = wait_time;
        JL_PLED->CNT_SYNC = sync;
        log_debug("sync = %d\n", sync);
    }

#if 0
    if (sync) {
        u32 dec = cur_cnt_max / sync;
        JL_PLED->CNT_DEC = dec > 255 ? 0 : dec;
        log_debug("dec = %d\n", dec);
    }
#endif

    return 0;
}



