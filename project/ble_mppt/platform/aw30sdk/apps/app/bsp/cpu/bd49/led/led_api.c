#include "led_api.h"
#include "pwm_led.h"
#include "gpio.h"
#include "app_config.h"



const led_board_cfg_t *led_board_cfg = NULL;


static void led_effect_output_by_hardware(led_pdata_t *led_effect)
{
    u32 time_unit = 50;
    pwm_led_pdata_t pled;
    memset(&pled, 0, sizeof(pwm_led_pdata_t));
    if (led_board_cfg->layout == ONE_IO_ONE_LED) {
        pled.port0 = led_board_cfg->led0.port;
        pled.port1 = -1;
        if (led_board_cfg->led0.logic == BRIGHT_BY_HIGH) {
            pled.first_logic = 0;
            pled.h_pwm_duty = led_board_cfg->led0.brightness;
        } else {
            pled.first_logic = 1;
            pled.l_pwm_duty = led_board_cfg->led0.brightness;
        }
        pled.alternate_out = 0;
    } else {
        pled.port0 = led_board_cfg->led0.port;
        pled.port1 = led_board_cfg->led1.port;
        pled.alternate_out = 0;
        if (led_effect->ctl_option == CTL_LED0_ONLY) {
            pled.port1 = -1;
            if (led_board_cfg->led0.logic == BRIGHT_BY_HIGH) {
                pled.first_logic = 0;
                pled.h_pwm_duty = led_board_cfg->led0.brightness;
            } else {
                pled.first_logic = 1;
                pled.l_pwm_duty = led_board_cfg->led0.brightness;
            }
        } else if (led_effect->ctl_option == CTL_LED1_ONLY) {
            pled.port0 = -1;
            if (led_board_cfg->led1.logic == BRIGHT_BY_HIGH) {
                pled.first_logic = 0;
                pled.h_pwm_duty = led_board_cfg->led1.brightness;
            } else {
                pled.first_logic = 1;
                pled.l_pwm_duty = led_board_cfg->led1.brightness;
            }
        } else if (led_effect->ctl_option == CTL_LED01_ASYNC) {
            pled.first_logic = 0;
            pled.alternate_out = 1;
            if (led_board_cfg->led0.logic == BRIGHT_BY_HIGH) {
                pled.h_pwm_duty = led_board_cfg->led0.brightness;
                pled.l_pwm_duty = led_board_cfg->led1.brightness;
            } else {
                pled.h_pwm_duty = led_board_cfg->led1.brightness;
                pled.l_pwm_duty = led_board_cfg->led0.brightness;
            }
        } else {
            pled.first_logic = 0;
            if (led_board_cfg->led0.logic != led_board_cfg->led1.logic) {
                pled.alternate_out = 1;
            }
            if (led_board_cfg->led0.logic == BRIGHT_BY_HIGH) {
                pled.h_pwm_duty = led_board_cfg->led0.brightness;
                pled.l_pwm_duty = led_board_cfg->led1.brightness;
            } else {
                pled.h_pwm_duty = led_board_cfg->led1.brightness;
                pled.l_pwm_duty = led_board_cfg->led0.brightness;
            }
        }
    }
    pled.out_mode = led_effect->ctl_mode;
    pled.ctl_cycle = led_effect->ctl_cycle * time_unit;
    pled.ctl_cycle_num = led_effect->ctl_cycle_num;
    pled.cbfunc = led_effect->cbfunc;
    switch (led_effect->ctl_mode) {
    case CYCLE_ONCE_BRIGHT:
        pled.out_once.pwm_out_time = led_effect->once_bright.bright_time * time_unit;
        pled.pwm_cycle = pled.out_once.pwm_out_time * 4;
        pled.pwm_cycle = pled.pwm_cycle > 320 ? 320 : pled.pwm_cycle;
        break;
    case CYCLE_TWICE_BRIGHT:
        pled.out_twice.first_pwm_out_time  = led_effect->twice_bright.first_bright_time * time_unit;
        pled.out_twice.second_pwm_out_time = led_effect->twice_bright.second_bright_time * time_unit;
        pled.out_twice.pwm_gap_time        = led_effect->twice_bright.bright_gap_time * time_unit;
        pled.pwm_cycle = pled.out_twice.first_pwm_out_time * 4;
        pled.pwm_cycle = pled.pwm_cycle > 320 ? 320 : pled.pwm_cycle;
        break;
    case CYCLE_BREATHE_BRIGHT:
        pled.out_breathe.pwm_out_time = led_effect->breathe_bright.bright_time * time_unit;
        pled.out_breathe.pwm_duty_max_keep_time = led_effect->breathe_bright.brightest_keep_time * time_unit;
        pled.pwm_cycle = pled.out_breathe.pwm_out_time * 4;
        pled.pwm_cycle = pled.pwm_cycle > 320 ? 320 : pled.pwm_cycle;
        break;
    case ALWAYS_BRIGHT:
        pled.ctl_cycle = 5;
        pled.pwm_cycle = 32;
        pled.out_mode = CYCLE_ONCE_BRIGHT;
        pled.out_once.pwm_out_time = pled.ctl_cycle;
        pled.cbfunc = NULL;
        break;
    case ALWAYS_EXTINGUISH:
        pwm_led_hw_close();
        return;
    }
    pwm_led_hw_init(&pled);
}

static void led_effect_output_by_software(led_pdata_t *led_effect)
{
    //暂不支持
}

void led_effect_info_dump(led_pdata_t *led_effect)
{
    printf("led0.port = %d", led_board_cfg->led0.port);
    printf("led0.logic = %d", led_board_cfg->led0.logic);
    printf("led0.brightness = %d", led_board_cfg->led0.brightness);
    printf("led1.port = %d", led_board_cfg->led1.port);
    printf("led1.logic = %d", led_board_cfg->led1.logic);
    printf("led1.brightness = %d", led_board_cfg->led1.brightness);
    printf("layout = %d", led_board_cfg->layout);
    printf("ctl_option = %d", led_effect->ctl_option);
    printf("ctl_mode = %d", led_effect->ctl_mode);
    printf("ctl_cycle = %d", led_effect->ctl_cycle);
    printf("ctl_cycle_num = %d", led_effect->ctl_cycle_num);
    printf("cbfunc = %d", (u32)led_effect->cbfunc);
    if (led_effect->ctl_mode == 0) {
        printf("once_bright.bright_time = %d", led_effect->once_bright.bright_time);
    } else if (led_effect->ctl_mode == 1) {
        printf("twice_bright.first_bright_time = %d", led_effect->twice_bright.first_bright_time);
        printf("twice_bright.bright_gap_time = %d", led_effect->twice_bright.bright_gap_time);
        printf("twice_bright.second_bright_time = %d", led_effect->twice_bright.second_bright_time);
    } else if (led_effect->ctl_mode == 2) {
        printf("breathe_bright.bright_time = %d", led_effect->breathe_bright.bright_time);
        printf("breathe_bright.brightest_keep_time = %d", led_effect->breathe_bright.brightest_keep_time);
    }
}


void led_effect_init(led_pdata_t *led_effect)
{
    /* led_effect_info_dump(led_effect); */
    if (led_effect->board_cfg) {
        led_board_cfg = led_effect->board_cfg;
    }

    if (led_board_cfg == NULL) {
        return;
    }

    u32 use_pwm_led;
    if (led_board_cfg->layout <= ONE_IO_TWO_LED) {
        use_pwm_led = 1;
    } else if ((led_effect->ctl_option == CTL_LED0_ONLY) || \
               (led_effect->ctl_option == CTL_LED1_ONLY)) {
        use_pwm_led = 1;
    } else {
        use_pwm_led = 0;
    }

    if (use_pwm_led) {
        led_effect_output_by_hardware(led_effect);
    } else {
        led_effect_output_by_software(led_effect);
    }

    if ((use_pwm_led) && (led_board_cfg->layout == THREE_IO_TWO_LED)) {//第三IO
        if (led_effect->ctl_mode == ALWAYS_EXTINGUISH) {
            gpio_set_mode(IO_PORT_SPILT(led_board_cfg->com_pole_port), PORT_HIGHZ);
        } else {
            gpio_set_mode(IO_PORT_SPILT(led_board_cfg->com_pole_port), PORT_OUTPUT_LOW);
        }
    }
}

void led_effect_output(led_pdata_t *led_effect)
{
    led_effect_init(led_effect);
}

void led_effect_board_init(const led_board_cfg_t *cfg)
{
    led_board_cfg = cfg;
}


