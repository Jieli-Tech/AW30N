#ifndef  __HW_TIMER_H__
#define  __HW_TIMER_H__

#include "cpu.h"
#include "config.h"
#include "gpio.h"
#include "clock.h"
#include "hwi.h"
#include "dma.h"

/* JL_TIMERx->CON */
//工作模式选择
#define TIMER_MODE_DISABLE      (0b00 << 0) //关闭TIMER功能
#define TIMER_MODE_TIME         (0b01 << 0) //定时、计数模式
#define TIMER_MODE_RISE         (0b10 << 0) //上升沿捕获
#define TIMER_MODE_FALL         (0b11 << 0) //下降沿捕获
//捕获模式端口选择
#define TIMER_CESL_IO           (0b00 << 2) //IO口输入
#define TIMER_CESL_IRFLT        (0b01 << 2) //IRFLT滤波
//预分频选择
#define TIMER_PSET_1			(0b0000 << 4)   //1分频
#define TIMER_PSET_4       	    (0b0001 << 4)	//4
#define TIMER_PSET_16      	    (0b0010 << 4)	//16
#define TIMER_PSET_64      	    (0b0011 << 4)	//64
#define TIMER_PSET_2     		(0b0100 << 4)	//2
#define TIMER_PSET_8     		(0b0101 << 4)	//8
#define TIMER_PSET_32    		(0b0110 << 4)	//32
#define TIMER_PSET_128   		(0b0111 << 4)	//128
#define TIMER_PSET_256   		(0b1000 << 4)	//256
#define TIMER_PSET_1024 		(0b1001 << 4)	//1024
#define TIMER_PSET_4096  		(0b1010 << 4)	//4096
#define TIMER_PSET_16384 		(0b1011 << 4)	//16384
#define TIMER_PSET_512 		    (0b1100 << 4)	//512
#define TIMER_PSET_2048 		(0b1101 << 4)	//2048
#define TIMER_PSET_8192		    (0b1110 << 4)	//8192
#define TIMER_PSET_32768		(0b1111 << 4)	//32768
//PWM使能
#define TIMER_PWM_EN		    (0b1 << 8)	//PWM输出使能
//PWM输出反向使能
#define TIMER_PWM_INV		    (0b1 << 9)	//PWM输出反向使能
//timer驱动源选择
#define TIMER_SSEL_1B1			(0b0000 << 10)	//关闭
#define TIMER_SSEL_LSB_CLK      (0b0001 << 10)	//LSB时钟
#define TIMER_SSEL_RC_250K      (0b0010 << 10)	//250K
#define TIMER_SSEL_RC_16M       (0b0011 << 10)	//RC16M
#define TIMER_SSEL_LRC_CLK      (0b0100 << 10)	//LRC
#define TIMER_SSEL_STD_12M      (0b0101 << 10)	//12M
#define TIMER_SSEL_STD_24M      (0b0110 << 10)	//24M
#define TIMER_SSEL_STD_48M      (0b0111 << 10)	//48M
#define TIMER_SSEL_CLK_OUT2IO   (0b1000 << 10)	//CLK_OUT
#define TIMER_SSEL_PAT_CLK      (0b1001 << 10)	//
#define TIMER_SSEL_RTC_OSC      (0b1010 << 10)	//RTC
// #define TIMER_SSEL_1B1,         (0b1011 << 10)	//关闭
// #define TIMER_SSEL_1B1,         (0b1100 << 10)	//关闭
// #define TIMER_SSEL_1B1,         (0b1101 << 10)	//关闭
// #define TIMER_SSEL_1B1,         (0b1110 << 10)	//关闭
#define TIMER_SSEL_TMRX_CIN     (0b1111 << 10)	//HW_TIMER
//清PND
#define TIMER_PCLR              (0b1 << 14) //清PND 只写位
//PND
#define TIMER_PND               (0b1 << 15) //清PND 只读位
//双边沿捕获使能
#define TIMER_DUAL_EDGE_EN      (0b1 << 16) //使能双边沿捕获模式,上升沿或下降沿捕获模式使能时有效

/* JL_TIMERx->CON */
//IRFLT模块使能
#define IRFLT_EN          (0b1 << 0)  //使能红外滤波功能
//IRFLT时钟源选择
#define IRFLT_TSRC_LSB_CLK      (0b00 << 2) //lsb_clk
// #define IRFLT_TSRC_LSB_CLK            (0b01 << 2) //lsb_clk
#define IRFLT_TSRC_STD_12M      (0b10 << 2) //12M
#define IRFLT_TSRC_STD_24M      (0b11 << 2) //24M
//IRFLT分频设置
#define IRFLT_PSEL_1            (0b0000 << 4)    //分频1
#define IRFLT_PSEL_2            (0b0001 << 4)    //分频2
#define IRFLT_PSEL_4            (0b0010 << 4)    //分频4
#define IRFLT_PSEL_8            (0b0011 << 4)    //分频8
#define IRFLT_PSEL_16           (0b0100 << 4)    //分频16
#define IRFLT_PSEL_32           (0b0101 << 4)    //分频32
#define IRFLT_PSEL_64           (0b0110 << 4)    //分频64
#define IRFLT_PSEL_128          (0b0111 << 4)    //分频128
#define IRFLT_PSEL_256          (0b1000 << 4)    //分频256
#define IRFLT_PSEL_512          (0b1001 << 4)    //分频512
#define IRFLT_PSEL_1024         (0b1010 << 4)    //分频1024
#define IRFLT_PSEL_2048         (0b1011 << 4)    //分频2048
#define IRFLT_PSEL_4096         (0b1100 << 4)    //分频4096
#define IRFLT_PSEL_8192         (0b1101 << 4)    //分频8192
#define IRFLT_PSEL_16384        (0b1110 << 4)    //分频16384
#define IRFLT_PSEL_32768        (0b1111 << 4)    //分频32768

static inline int get_hw_timerx_num(const JL_TIMER_TypeDef *timerx)
{
    return (u32)((timerx - JL_TIMER0) / (JL_TIMER1 - JL_TIMER0));
}

void hw_timer_init(JL_TIMER_TypeDef *JL_TIMERx, u32 msec);
void hw_timer_pwm_init(JL_TIMER_TypeDef *JL_TIMERx, u32 gpio, u32 freq, u32 duty);
void hw_timer_pwm_set_duty(JL_TIMER_TypeDef *JL_TIMERx, u32 duty);
void hw_timer_capture_init(JL_TIMER_TypeDef *JL_TIMERx, u32 gpio);

#endif
