#ifndef _LPCTMU_H_
#define _LPCTMU_H_

#include "gpio.h"


#define LPCTMU_CH_SIZE  8

enum lpctmu_clk_src {
    LPCTMU_LRC_200K,
    LPCTMU_STD_24M,
};

enum lpctmu_ctl_mode {
    SOFT_SWITCH_MODE,   //软件切换模式
    HW_NORMAL_POLL_MODE,//硬件自动轮询模式
    HW_UDMA_POLL_MODE,  //硬件dma自动轮询模式
};

struct ch_en_cfg {
    u8 ch0: 1;
    u8 ch1: 1;
    u8 ch2: 1;
    u8 ch3: 1;
    u8 ch4: 1;
    u8 ch5: 1;
    u8 ch6: 1;
    u8 ch7: 1;
};

struct lpctmu_platform_data {
    u8 sample_time_ms;
    u16 aim_vol_delta;
    u16 aim_charge_khz;
    enum lpctmu_clk_src clk_sel;
    enum lpctmu_ctl_mode ctl_mode;
    union {
        u8 val;
        struct ch_en_cfg cfg;
    } ch_en;
};

#define LPCTMU_PLATFORM_DATA_BEGIN(data) \
    static const struct lpctmu_platform_data data = {

#define LPCTMU_PLATFORM_DATA_END() \
};

void lpctmu_init(const struct lpctmu_platform_data *pdata);
void lpctmu_kstart(void);
u32 get_lpctmu_value(u8 ch);

#endif

