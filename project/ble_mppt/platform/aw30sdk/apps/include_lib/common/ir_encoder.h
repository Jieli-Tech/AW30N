#ifndef  __IR_ENCODER_H__
#define  __IR_ENCODER_H__

#include "cpu.h"
enum {
    IR_STATUS_IDLE,
    IR_STATUS_REPEAT,
    IR_STATUS_HEAD,
    IR_STATUS_DATA,
    IR_STATUS_END,
};

void ir_encoder_init(u32 gpio, u32 freq, u32 duty); //gpio:发送脚, freq:载波频率, duty:占空比,满量程10000
u32 ir_encode_tx(u8 ir_addr, u8 ir_cmd, u8 repeat_en); //addr:地址码, cmd:命令码, repeat_en:重复码发送使能
u32 ir_encode_repeat_stop();

#endif
