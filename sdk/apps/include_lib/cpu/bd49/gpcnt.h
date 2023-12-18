#ifndef _GPCNT_H
#define _GPCNT_H

#include "typedef.h"

enum gpcnt_gss_css_src {
    GPCNT_SRC_ALWAY1,//disable
    GPCNT_SRC_SYS_PLL_D1P0,
    GPCNT_SRC_RC16M,
    GPCNT_SRC_RC250K,
    GPCNT_SRC_RTC_OSC,
    GPCNT_SRC_LRC_CLK,
    GPCNT_SRC_PAT_CLK,
    GPCNT_SRC_BTOSC_24M,
    GPCNT_SRC_BTOSC_48M,
    GPCNT_SRC_HSB_CLK,
    GPCNT_SRC_LSB_CLK,
    GPCNT_SRC_PLL_96M,
    GPCNT_SRC_STD_24M,
    GPCNT_SRC_STD_48M,
    GPCNT_SRC_XOSC_FSCK,
    GPCNT_SRC_P33_CLK_DBG,
    GPCNT_SRC_ASS_DBG_CLKO,
    GPCNT_SRC_CAP_MUX_OUT,
    GPCNT_SRC_CLK_MUX_IN,
    GPCNT_SRC_IRFLT_IN,
    GPCNT_SRC_LCTM0_ANA_CLK,
};

#define GPCNT_MAX_HW_NUM                 1
typedef enum {
    GPCNT0,
    GPCNT_MAX,
} gpcnt_dev;


#define gpcnt_enable(reg)                ((reg)->CON |= BIT(0))
#define gpcnt_disable(reg)               ((reg)->CON &= ~BIT(0))
#define gpcnt_pnd(reg)                   ((reg)->CON & BIT(31))
#define gpcnt_clr_pnd(reg)               ((reg)->CON |= BIT(30))
#define gpcnt_gts(reg, val)              SFR((reg)->CON, 16, 4, val)
#define gpcnt_gss(reg, val)              SFR((reg)->CON, 8, 5, val) //
#define gpcnt_css(reg, val)              SFR((reg)->CON, 1, 5, val) //
#define gpcnt_w_reg_con(reg, val)        ((reg)->CON = (val))
#define gpcnt_r_reg_con(reg)             ((reg)->CON)
#define gpcnt_r_reg_num(reg)             ((reg)->NUM)

//test_time_ms:测试耗时(ms)
//xss_src:待测试的内部时钟
//return:待测试的内部时钟频率(hz)
u32 gpcnt_trim_inside_clk(gpcnt_dev gpcnt, u32 test_time_ms, enum gpcnt_gss_css_src xss_src);
//test_time_ms:测试耗时(400ms)
//test_clk_freq:待测试的外部时钟粗略频率(hz)(未知写0)
//test_gpio:待测试的外部时钟输入引脚
//return:待测试的外部时钟精确频率(hz)
u32 gpcnt_trim_outside_clk(gpcnt_dev gpcnt, u32 test_time_ms, u32 test_clk_freq, u8 test_gpio);
#endif

