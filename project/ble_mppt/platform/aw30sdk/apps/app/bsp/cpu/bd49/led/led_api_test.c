#include "led_api.h"


// *INDENT-OFF*

//ALL OFF
LED_PLATFORM_DATA_BEGIN(led_data0)
    .ctl_option = CTL_LED01_SYNC,
    .ctl_mode = ALWAYS_EXTINGUISH,
LED_PLATFORM_DATA_END()

//LED0 ON
LED_PLATFORM_DATA_BEGIN(led_data1)
    .ctl_option = CTL_LED0_ONLY,
    .ctl_mode = ALWAYS_BRIGHT,
LED_PLATFORM_DATA_END()

//LED1 ON
LED_PLATFORM_DATA_BEGIN(led_data2)
    .ctl_option = CTL_LED1_ONLY,
    .ctl_mode = ALWAYS_BRIGHT,
LED_PLATFORM_DATA_END()

//ALL ON
LED_PLATFORM_DATA_BEGIN(led_data3)
    .ctl_option = CTL_LED01_SYNC,
    .ctl_mode = ALWAYS_BRIGHT,
LED_PLATFORM_DATA_END()

//LED0 2s闪1次0.1s
LED_PLATFORM_DATA_BEGIN(led_data4)
    .ctl_option = CTL_LED0_ONLY,
    .ctl_mode = CYCLE_ONCE_BRIGHT,
    .ctl_cycle = 40,//*50 =2000
    .ctl_cycle_num = 0,
    .once_bright.bright_time = 2,//*50=100
LED_PLATFORM_DATA_END()

//LED1 2s闪1次0.1s
LED_PLATFORM_DATA_BEGIN(led_data5)
    .ctl_option = CTL_LED1_ONLY,
    .ctl_mode = CYCLE_ONCE_BRIGHT,
    .ctl_cycle = 40,//*50 =2000
    .ctl_cycle_num = 0,
    .once_bright.bright_time = 2,//*50=100
LED_PLATFORM_DATA_END()

//LED0 LED1  2s交替闪1次0.1s
LED_PLATFORM_DATA_BEGIN(led_data6)
    .ctl_option = CTL_LED01_ASYNC,
    .ctl_mode = CYCLE_ONCE_BRIGHT,
    .ctl_cycle = 40,//*50 =2000
    .ctl_cycle_num = 0,
    .once_bright.bright_time = 2,//*50=100
LED_PLATFORM_DATA_END()

//LED0 2s闪2次0.1s
LED_PLATFORM_DATA_BEGIN(led_data7)
    .ctl_option = CTL_LED0_ONLY,
    .ctl_mode = CYCLE_TWICE_BRIGHT,
    .ctl_cycle = 40,//*50 =2000
    .ctl_cycle_num = 0,
    .twice_bright.first_bright_time = 2,//*50=100
    .twice_bright.bright_gap_time = 2,//*50=100
    .twice_bright.second_bright_time = 2,//*50=100
LED_PLATFORM_DATA_END()

//LED1 2s闪2次0.1s
LED_PLATFORM_DATA_BEGIN(led_data8)
    .ctl_option = CTL_LED1_ONLY,
    .ctl_mode = CYCLE_TWICE_BRIGHT,
    .ctl_cycle = 40,//*50 =2000
    .ctl_cycle_num = 0,
    .twice_bright.first_bright_time = 2,//*50=100
    .twice_bright.bright_gap_time = 2,//*50=100
    .twice_bright.second_bright_time = 2,//*50=100
LED_PLATFORM_DATA_END()

//LED0 LED1  2s交替闪2次0.1s
LED_PLATFORM_DATA_BEGIN(led_data9)
    .ctl_option = CTL_LED01_ASYNC,
    .ctl_mode = CYCLE_TWICE_BRIGHT,
    .ctl_cycle = 40,//*50 =2000
    .ctl_cycle_num = 0,
    .twice_bright.first_bright_time = 2,//*50=100
    .twice_bright.bright_gap_time = 2,//*50=100
    .twice_bright.second_bright_time = 2,//*50=100
LED_PLATFORM_DATA_END()

//LED0 5s闪呼吸1次3s
LED_PLATFORM_DATA_BEGIN(led_data10)
    .ctl_option = CTL_LED0_ONLY,
    .ctl_mode = CYCLE_BREATHE_BRIGHT,
    .ctl_cycle = 100,//*50 =5000
    .ctl_cycle_num = 0,
    .breathe_bright.bright_time = 60,//*50=3000
    .breathe_bright.brightest_keep_time = 20,//*50=1000
LED_PLATFORM_DATA_END()

//LED1 5s闪呼吸1次3s
LED_PLATFORM_DATA_BEGIN(led_data11)
    .ctl_option = CTL_LED1_ONLY,
    .ctl_mode = CYCLE_BREATHE_BRIGHT,
    .ctl_cycle = 100,//*50 =5000
    .ctl_cycle_num = 0,
    .breathe_bright.bright_time = 60,//*50=3000
    .breathe_bright.brightest_keep_time = 20,//*50=1000
LED_PLATFORM_DATA_END()

//LED0 LED1  5s交替呼吸1次3s
LED_PLATFORM_DATA_BEGIN(led_data12)
    .ctl_option = CTL_LED01_ASYNC,
    .ctl_mode = CYCLE_BREATHE_BRIGHT,
    .ctl_cycle = 100,//*50 =5000
    .ctl_cycle_num = 0,
    .breathe_bright.bright_time = 60,//*50=3000
    .breathe_bright.brightest_keep_time = 20,//*50=1000
LED_PLATFORM_DATA_END()


//单IO单灯
const led_board_cfg_t led_cfg0 = {
    .layout = ONE_IO_ONE_LED,
    .led0.port = IO_PORTA_03,
    .led0.logic = BRIGHT_BY_HIGH,
    .led0.brightness = 80,
};

//单IO双灯
const led_board_cfg_t led_cfg1 = {
    .layout = ONE_IO_TWO_LED,
    .led0.port = IO_PORTA_03,
    .led1.port = IO_PORTA_03,
    .led0.logic = BRIGHT_BY_HIGH,
    .led1.logic = BRIGHT_BY_LOW,
    .led0.brightness = 80,
    .led1.brightness = 88,
};

//双IO双灯
const led_board_cfg_t led_cfg2 = {
    .layout = TWO_IO_TWO_LED,
    .led0.port = IO_PORTA_02,
    .led1.port = IO_PORTA_03,
    .led0.logic = BRIGHT_BY_HIGH,
    .led1.logic = BRIGHT_BY_HIGH,
    .led0.brightness = 80,
    .led1.brightness = 80,
};


void led_api_test(void)
{
    printf("******** led api test *************\n");

    /* led_effect_board_init(&led_cfg0); */
    led_effect_board_init(&led_cfg1);
    /* led_effect_board_init(&led_cfg2); */


    /* led_effect_output(&led_data0); */
    /* led_effect_output(&led_data1); */
    /* led_effect_output(&led_data2); */
    /* led_effect_output(&led_data3); */
    /* led_effect_output(&led_data4); */
    /* led_effect_output(&led_data5); */
    /* led_effect_output(&led_data6); */
    /* led_effect_output(&led_data7); */
    /* led_effect_output(&led_data8); */
    led_effect_output(&led_data9);
    /* led_effect_output(&led_data10); */
    /* led_effect_output(&led_data11); */
    /* led_effect_output(&led_data12); */

#if 1
    extern void wdt_clear();
    while (1) {
        wdt_clear();
    }
#endif
}


