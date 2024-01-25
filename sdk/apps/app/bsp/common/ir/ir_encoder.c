#include "ir_encoder.h"
#include "gptimer.h"

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[ir_encode]"
#include "log.h"

#define IR_NEC_PULSE_UNIT   563
#define IR_NEC_BIT_1_H  1
#define IR_NEC_BIT_1_L  3
#define IR_NEC_BIT_0_H  1
#define IR_NEC_BIT_0_L  1
#define IR_NEC_HEAD_H   16
#define IR_NEC_HEAD_L   8
#define IR_NEC_END_H    1
#define IR_NEC_END_L    10
#define IR_NEC_END_REPEAT_L    196
#define IR_NEC_REPEAT_H 16
#define IR_NEC_REPEAT_L 4
#define IR_NEC_REPEAT   0x00FF00FF

static struct gptimer_pwm_config *pwm_config = NULL;
static struct gptimer_pwm_config _pwm_config;
static int pwm_tid;
static struct gptimer_config *timer_config = NULL;
static struct gptimer_config _timer_config;
static int timer_tid;

static u8 cnt_h, cnt_l, cnt, ir_status, repeat_flag;
static u32 ir_data, _ir_data;
static void ir_pwm_ctrl()
{
    if (cnt_h) {
        gptimer_pwm_enable(pwm_tid);
        cnt_h--;
    } else {
        if (cnt_l) {
            gptimer_pwm_disable(pwm_tid);
            cnt_l--;
        } else {
            /* _gptimer_pwm_enable(pwm_tid); */
        }
    }
}
static void ir_encode_callback(int tid)
{
    ir_pwm_ctrl();
    switch (ir_status) {
    case IR_STATUS_IDLE:
        gptimer_pwm_disable(pwm_tid);
        gptimer_pause(timer_tid);
        break;
    case IR_STATUS_REPEAT:
        if ((cnt_h == 0) && (cnt_l == 0)) {
            cnt_h = IR_NEC_END_H;
            cnt_l = IR_NEC_END_L;
            if (repeat_flag) {
                cnt_l = IR_NEC_END_REPEAT_L;
            }
            ir_status = IR_STATUS_END;
        }
        break;
    case IR_STATUS_HEAD:
        if ((cnt_h == 0) && (cnt_l == 0)) {
            cnt = 1;
            if (_ir_data & BIT(0)) {
                cnt_h = IR_NEC_BIT_1_H;
                cnt_l = IR_NEC_BIT_1_L;
            } else {
                cnt_h = IR_NEC_BIT_0_H;
                cnt_l = IR_NEC_BIT_0_L;
            }
            ir_status = IR_STATUS_DATA;
            _ir_data >>= 1;
        }
        break;
    case IR_STATUS_DATA:
        if ((cnt_h == 0) && (cnt_l == 0)) {
            if (cnt >= 32) {
                cnt_h = IR_NEC_END_H;
                cnt_l = IR_NEC_END_L;
                if (repeat_flag) {
                    cnt_l = IR_NEC_END_REPEAT_L;
                }
                ir_status = IR_STATUS_END;
            } else {
                cnt++;
                if (_ir_data & BIT(0)) {
                    cnt_h = IR_NEC_BIT_1_H;
                    cnt_l = IR_NEC_BIT_1_L;
                } else {
                    cnt_h = IR_NEC_BIT_0_H;
                    cnt_l = IR_NEC_BIT_0_L;
                }
                ir_status = IR_STATUS_DATA;
                _ir_data >>= 1;
            }
        }
        break;
    case IR_STATUS_END:
        if ((cnt_h == 0) && (cnt_l == 0)) {
            if (repeat_flag) {
                cnt_h = IR_NEC_REPEAT_H;
                cnt_l = IR_NEC_REPEAT_L;
                ir_status = IR_STATUS_REPEAT;
            } else {
                ir_status = IR_STATUS_IDLE;
            }
        }
        break;
    default:
        break;
    }
}

void ir_encoder_init(u32 gpio, u32 freq, u32 duty)
{
    log_info("func:%s(), line:%d\n", __func__, __LINE__);
    pwm_config = &_pwm_config;
    pwm_config->port = gpio / IO_GROUP_NUM; //PORTA;
    pwm_config->pin = gpio % IO_GROUP_NUM; //3;
    pwm_config->freq = freq;
    pwm_config->pwm_duty_X10000 = duty;
    pwm_config->tid = -1;
    pwm_tid = gptimer_pwm_init(pwm_config);
    log_info("pwm_config tid = %d\n", pwm_tid);
    gpio_set_mode(IO_PORT_SPILT(gpio), PORT_OUTPUT_LOW);
    gptimer_start(pwm_tid);
    gptimer_pwm_disable(pwm_tid);

    timer_config = &_timer_config;
    timer_config->resolution_us = IR_NEC_PULSE_UNIT;
    timer_config->irq_cb = ir_encode_callback;
    timer_config->tid = -1;
    timer_config->irq_priority = 7; //使用不可屏蔽中断
    timer_tid = gptimer_init(timer_config);
    log_info("timer_config tid = %d\n", timer_tid);
}

u32 ir_encode_tx(u8 ir_addr, u8 ir_cmd, u8 repeat_en)
{
    if (ir_status != IR_STATUS_IDLE) {
        return -1;
    }
    u32 user_data;
    u32 _user_data;
    user_data = (ir_addr << 24) + ((0xbf << 16) & 0xff0000) + (ir_cmd << 8) + ((~ir_cmd) & 0xff);
    ir_data = user_data;
    _user_data = ((~ir_cmd << 24) & 0xff000000) + (ir_cmd << 16) + ((0xbf << 8) & 0xff00) + (ir_addr);
    _ir_data = _user_data;
    cnt_h = IR_NEC_HEAD_H;
    cnt_l = IR_NEC_HEAD_L;
    ir_status = IR_STATUS_HEAD;
    repeat_flag = repeat_en;
    log_info("ir_data 0x%08x\n", ir_data);
    log_info("_ir_data 0x%08x\n", _ir_data);

    gptimer_start(timer_tid);
    return 0;
}

u32 ir_encode_repeat_stop()
{
    repeat_flag = 0;
    return 0;
}

