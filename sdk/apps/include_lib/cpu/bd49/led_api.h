#ifndef __LED_API_H__
#define __LED_API_H__


#include "typedef.h"
#include "gpio.h"


enum led_layout_mode {              //根据原理图选择
    ONE_IO_ONE_LED,                 //单IO单灯
    ONE_IO_TWO_LED,                 //单IO双灯
    TWO_IO_TWO_LED,                 //双IO双灯
    THREE_IO_TWO_LED,               //三IO双灯, 即两灯的阴极都连到第三IO
};
enum led_logic_mode {
    BRIGHT_BY_LOW,                  //给高电平亮
    BRIGHT_BY_HIGH,                 //给高电平亮
};
enum led_ctl_option {
    CTL_LED0_ONLY,                  //只控led0
    CTL_LED1_ONLY,                  //只控led1
    CTL_LED01_ASYNC,                //led0&led1异步(交替)
    CTL_LED01_SYNC,                 //led0&led1同步
};
enum led_ctl_mode {
    CYCLE_ONCE_BRIGHT,              //周期单闪
    CYCLE_TWICE_BRIGHT,             //周期双闪
    CYCLE_BREATHE_BRIGHT,           //周期呼吸
    ALWAYS_BRIGHT,                  //常亮
    ALWAYS_EXTINGUISH,              //常灭
};

typedef struct one_led_platform_data {
    u8 port;                        //控灯IO
    u8 logic;                       //参考枚举led_logic_mode, 0：给高电平亮， 1：给低电平亮
    u8 brightness;                  //灯的亮度0~100
} one_led_pdata_t;

typedef struct led_board_cfg_data {
    one_led_pdata_t led0;
    one_led_pdata_t led1;
    u8 layout;                      //参考枚举led_layout_mode，填IO_PORTA_00 ~ (IO_PORT_MAX-1)为第三IO
    u8 com_pole_port;               //layout选择三IO双灯时有效，即两个灯的阴极都连到了这个IO
} led_board_cfg_t;

typedef struct led_platform_data {
    const led_board_cfg_t *board_cfg;
    u8 ctl_option;                  //参考枚举led_ctl_option
    u8 ctl_mode;                    //参考枚举led_ctl_mode
    u8 ctl_cycle;                   //控制周期, 单位50ms, 比如每5s闪一次灯，那么5s就是控制周期
    u8 ctl_cycle_num;               //控制周期的个数，值为0时，则控制周期无限循环，值为n时，则第n次控制周期之后,灯自动关闭
    union {
        struct {                    //周期单闪, 如灯每5s闪一次，每次亮100ms
            u8 bright_time;         //灯亮的时间，单位50ms
        } once_bright;
        struct {                    //周期双闪, 如灯每5s闪两次，第一次亮100ms，间隔50ms，再第二次亮100ms
            u8 first_bright_time;   //第一次灯亮的时间，单位50ms
            u8 bright_gap_time;     //间隔时间，单位50ms
            u8 second_bright_time;  //第二次灯亮的时间，单位50ms
        } twice_bright;
        struct {                    //周期呼吸，如灯5s呼吸一次，每次呼吸亮2s
            u8 bright_time;         //灯亮的时间，单位50ms，等于 占空比自增自减的时间 + 最大占空比保持的时间
            u8 brightest_keep_time; //亮度增到最大的时候，如果需要保持的时间，单位50ms，该时间要小于 bright_time
        } breathe_bright;
    };
    void (*cbfunc)(u32 cnt);        //灯效结束回调函数
} led_pdata_t;



#define LED_PLATFORM_DATA_BEGIN(data) \
    led_pdata_t data = {

#define LED_PLATFORM_DATA_END()  \
    };



void led_effect_board_init(const led_board_cfg_t *cfg);

void led_effect_output(led_pdata_t *led_effect);



#endif


