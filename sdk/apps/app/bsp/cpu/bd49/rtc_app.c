#if 0
#include "power_interface.h"

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[rtc]"
#include "log.h"

const struct sys_time def_sys_time = {  //初始一下当前时间
    .year = 2020,
    .month = 1,
    .day = 1,
    .hour = 0,
    .min = 0,
    .sec = 0,
};

const struct sys_time def_alarm = {     //初始一下目标时间，即闹钟时间
    .year = 2050,
    .month = 1,
    .day = 1,
    .hour = 0,
    .min = 0,
    .sec = 0,
};

/* extern void alarm_isr_user_cbfun(u8 index); */
RTC_DEV_PLATFORM_DATA_BEGIN(rtc_data)
.default_sys_time = &def_sys_time,
 .default_alarm = &def_alarm,
  .clk_sel = CLK_SEL_32K,
   .cbfun = NULL,                      //闹钟中断的回调函数,用户自行定义
    /* .cbfun = alarm_isr_user_cbfun, */
    RTC_DEV_PLATFORM_DATA_END()


    void rtc_demo_init()
{
    struct sys_time time;

    rtc_init(&rtc_data);

    while (1) {
        void mdelay(u32 ms);
        mdelay(1000);
        read_sys_time(&time);
        printf("time->%d.%d.%d %d.%d.%d\n", time.year, time.month, time.day, time.hour, time.min, time.sec);
    }
}


#endif



