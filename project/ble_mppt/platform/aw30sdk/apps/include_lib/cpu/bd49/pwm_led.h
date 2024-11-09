#ifndef __PWM_LED_H__
#define __PWM_LED_H__


#include "typedef.h"


/****************************************************************************************************************

模块说明 ：（必看）

    通常我们对PWM的认知是：就一种PWM波形，在一个周期内，由高电平和低电平组成，它们所占时间比例可调。
    但是，LED_PWM不是这样的，他有两种PWM波形:
    第一种PWM波形是：在一个周期内，由高电平和高阻态组成，它们所占时间比例可调。在此命名为 h_pwm
    第二种PWM波形是：在一个周期内，由低电平和高阻态组成，它们所占时间比例可调。在此命名为 l_pwm

    高阻态可能是不定的电平，它的电平由外部电路决定，高阻态不会对外围电路起作用。比如接led灯，高阻态不会使灯亮的

******************************************************************************************************************/

// *INDENT-OFF*
typedef struct pwm_led_platform_data {
    u8 port0;           //输出波形的引脚
    u8 port1;           //输出波形的引脚
    u8 first_logic;     //0,先送出h_pwm     1:先送出l_pwm
    u8 alternate_out;   //范围0~7，两种波形(h_pwm和l_pwm)交替输出，每种波形输出的周期个数。当值为0时：则关闭交替输出模式，那么波形由first_logic决定。
    u32 pwm_cycle;       //pwm的周期，单位10us。
    u32 ctl_cycle;      //控制周期, 单位ms。比如每5s输出一次0.1s的pwm，那么5s就是控制周期
    u8 ctl_cycle_num;   //控制周期的个数，值为0时，则控制周期无限循环，  值为n时， 则第n次控制周期之后，模块自动关闭
    u8 h_pwm_duty;      //h_pwm 占空比0~100, 呼吸模式时，为最大占空比
    u8 l_pwm_duty;      //l_pwm 占空比0~100, 呼吸模式时，为最大占空比
    u8 out_mode;        //0：占空比固定模式, 一个控制周期输出一次pwm   1：占空比固定模式，一个控制周期输出两次pwm   2：占空比呼吸变化模式
    union {
        //固定波形输出模式,一个控制周期内只输出一次pwm
        struct {
            u32 pwm_out_time;   //输出pwm的时间，单位ms
        } out_once;
        //固定波形输出模式,一个控制周期内输出两次pwm
        struct {
            u32 first_pwm_out_time;     //第一次输出pwm的时间,单位ms
            u32 second_pwm_out_time;    //第二次输出pwm的时间,单位ms
            u32 pwm_gap_time;           //第一次和第二次的间隙时间,单位ms
        } out_twice;
        //呼吸变化输出模式，即PWM的占空比是变化的
        struct {
            u32 pwm_out_time;           //输出pwm的时间，单位ms, 等于 占空比自增自减的时间 + 最大占空比保持的时间
            u32 pwm_duty_max_keep_time; //pwm占空比增到最大的时候， 至少维持的时间，单位ms, 该时间要小于 pwm_out_time
        } out_breathe;
    };
    void (*cbfunc)(u32 cnt);   //中断回调函数
} pwm_led_pdata_t;


struct pwm_led_status_t {
    u8 dir;
    u8 level;
    u16 cur_cnt;
    u16 cnt_max;
    u16 next_cnt_max;
    u32 cnt_unit;
};



#define PWM_LED_PLATFORM_DATA_BEGIN(data) \
    struct pwm_led_platform_data data = {

#define PWM_LED_PLATFORM_DATA_END()  \
    };



void pwm_led_hw_init(void *pdata);

void pwm_led_hw_close(void);

void pwm_led_io_mount(void);

void pwm_led_io_unmount(void);

u32 pwm_led_is_working(void);

void pwm_led_get_sync_status(struct pwm_led_status_t *status);

u32 pwm_led_set_sync(struct pwm_led_status_t *status, u32 how_long_ago, u32 *sync_time);

#endif


