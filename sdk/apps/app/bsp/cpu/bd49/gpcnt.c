#pragma bss_seg(".gpcnt.data.bss")
#pragma data_seg(".gpcnt.data")
#pragma const_seg(".gpcnt.text.const")
#pragma code_seg(".gpcnt.text")
#pragma str_literal_override(".gpcnt.text.const")

#include "gpcnt.h"
#include "gpio.h"
#include "clock.h"

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[gpcnt]"
#include "log.h"


static JL_GPCNT_TypeDef *const gpcnt_regs[GPCNT_MAX_HW_NUM] = {
    JL_GPCNT,
    /* JL_GPCNT1, */
};

AT(.gpcnt.text.cache.L1)
static u32 gpcnt_get_num(gpcnt_dev gpcnt, enum gpcnt_gss_css_src gss_src, char n, enum gpcnt_gss_css_src css_src)
{
    /* ASSERT(gpcnt < GPCNT_MAX_HW_NUM, "params gpcnt error!"); */
    gpcnt_w_reg_con(gpcnt_regs[gpcnt], BIT(30));
    u32 gpcnt_num = 0;
    u16 cnt_return = 0;
    gpcnt_clr_pnd(gpcnt_regs[gpcnt]);//clr
    gpcnt_gss(gpcnt_regs[gpcnt], gss_src);
    gpcnt_gts(gpcnt_regs[gpcnt], n);//32*2^n
    gpcnt_css(gpcnt_regs[gpcnt], css_src);//次时钟
    gpcnt_clr_pnd(gpcnt_regs[gpcnt]);//clr
    gpcnt_enable(gpcnt_regs[gpcnt]);//en
    while (!(gpcnt_pnd(gpcnt_regs[gpcnt]))) {
        cnt_return++;
        mdelay(2);//需准确的2ms
        if (cnt_return > 1000) { //2s未完成,失败退出
            gpcnt_w_reg_con(gpcnt_regs[gpcnt], BIT(30));
            return 0;
        }
    }
    gpcnt_num = gpcnt_r_reg_num(gpcnt_regs[gpcnt]);
    gpcnt_w_reg_con(gpcnt_regs[gpcnt], BIT(30));

    return gpcnt_num;
}

AT(.gpcnt.text.cache.L1)
u32 gpcnt_trim_inside_clk(gpcnt_dev gpcnt, u32 test_time_ms, enum gpcnt_gss_css_src xss_src)
{
    /* ASSERT(gpcnt < GPCNT_MAX_HW_NUM, "params gpcnt error!"); */
    enum gpcnt_gss_css_src gss_src = xss_src;
    enum gpcnt_gss_css_src css_src = GPCNT_SRC_PLL_96M;
    u32 test_clk_freq = 0;
    if (xss_src == GPCNT_SRC_RC16M) {
        test_clk_freq = 16000000;
    } else if (xss_src == GPCNT_SRC_RC250K) {
        test_clk_freq = 250000;
    } else if (xss_src == GPCNT_SRC_LRC_CLK) {
        test_clk_freq = 200000;
    } else if (xss_src == GPCNT_SRC_RTC_OSC) {
        /* test_clk_freq = 24000000; */
    } else if (xss_src == GPCNT_SRC_BTOSC_24M) {
        test_clk_freq = 24000000;
    } else if (xss_src == GPCNT_SRC_BTOSC_48M) {
        test_clk_freq = 48000000;
    }

    char n = 15;
    if (test_clk_freq != 0) {
        u32 gss_period = (u32)(test_time_ms / 32.0f * test_clk_freq / 1000.0f);
        n = -1;
        while (gss_period) {
            gss_period >>= 1;
            n++;
        }
        if (n > 15) {
            n = 15;
        }
    }

    /* log_info("gpcnt trim clk start:"); */
    u32 knowm_clk = 96000000;//clk_get("lsb");
    u32 x_clk = 0;
    u32 num = gpcnt_get_num(gpcnt, gss_src, n, css_src);
    if (num != 0) {
        x_clk = (u32)(knowm_clk * 32.0f  / num * (1 << n));
        /* double cal_clk = knowm_clk * num / ((1 << n) * 32); */
    }
    /* log_info("knowm_clk:%d,n:%d, num:%d,x_clk:%d", knowm_clk, n, num, x_clk); */
    return x_clk;
}

u32 gpcnt_trim_outside_clk(gpcnt_dev gpcnt, u32 test_time_ms, u32 test_clk_freq, u8 test_gpio)
{
    /* ASSERT(gpcnt < GPCNT_MAX_HW_NUM, "params gpcnt error!"); */
    enum gpcnt_gss_css_src gss_src = GPCNT_SRC_CLK_MUX_IN;//GPCNT_SRC_IRFLT_IN;
    enum gpcnt_gss_css_src css_src = GPCNT_SRC_PLL_96M;
    char n = 9;
    if (test_clk_freq != 0) {
        u32 gss_period = (u32)(test_time_ms / 32.0f * test_clk_freq / 1000.0f);
        n = -1;
        while (gss_period) {
            gss_period >>= 1;
            n++;
        }
        if (n > 15) {
            n = 15;
        }
    }

    /* log_info("gpcnt trim clk start:"); */
    gpio_set_mode(IO_PORT_SPILT(test_gpio), PORT_INPUT_FLOATING);
    gpio_set_function(IO_PORT_SPILT(test_gpio), PORT_FUNC_CLK_PIN);

    u32 knowm_clk = 96000000;//clk_get("lsb");
    u32 x_clk = 0;
    u32 num = gpcnt_get_num(gpcnt, gss_src, n, css_src);
    if (num != 0) {
        x_clk = (u32)(knowm_clk * 32.0f  / num * (1 << n));
        /* u32 x_clk = knowm_clk * num / ((1 << n) * 32); */
    }
    /* log_info("knowm_clk:%d,n:%d, num:%d,x_clk:%d", knowm_clk, n, num, x_clk); */
    gpio_disable_function(IO_PORT_SPILT(test_gpio), PORT_FUNC_CLK_PIN);
    return x_clk;
}








/******************************test*****************************/
#if 0
void wdt_clear();
#define GPCNTX GPCNT0
void gpcnt_test()
{
    printf("***********gpcnt test**********\n");
#if 1//内部测试

    printf("*****RC16M:%d\n", gpcnt_trim_inside_clk(GPCNTX, 400, GPCNT_SRC_RC16M));
    printf("*****RC250k:%d\n", gpcnt_trim_inside_clk(GPCNTX, 400, GPCNT_SRC_RC250K));
    printf("*****LRC_CLK:%d\n", gpcnt_trim_inside_clk(GPCNTX, 400, GPCNT_SRC_LRC_CLK));
    printf("*****OSC24M:%d\n", gpcnt_trim_inside_clk(GPCNTX, 400, GPCNT_SRC_BTOSC_24M));
    printf("*****OSC48M:%d\n", gpcnt_trim_inside_clk(GPCNTX, 400, GPCNT_SRC_BTOSC_48M));
    printf("*****STD24M:%d\n", gpcnt_trim_inside_clk(GPCNTX, 400, GPCNT_SRC_STD_24M));
    printf("*****STD48M:%d\n", gpcnt_trim_inside_clk(GPCNTX, 400, GPCNT_SRC_STD_48M));
    /* #else//外部测试 */
    u32 cal_clk = 0, old_clk = 0;
    while (1) {
        cal_clk = gpcnt_trim_outside_clk(GPCNTX, 200, 0, IO_PORTB_04);
        if (cal_clk != old_clk) {
            printf("---------%d\n", cal_clk);
        }
        old_clk = cal_clk;
        mdelay(200);
        wdt_clear();
    }
#endif
}

#endif

