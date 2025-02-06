#ifndef __RF_RADIO_H__
#define __RF_RADIO_H__

#include "rf_send_queue.h"
#include "audio_rf_mge.h"


//--------------------半双工应用模式--------------------------
enum __attribute__((packed)) RFR_HALF_MODE {
    RFR_HALF_IDLE_MODE = 0,
    RFR_HALF_SEND_MODE,
    RFR_HALF_RECIVE_MODE,
    RFR_HALF_EXIT,
}; //对讲机工作模式

//--------------------全双工应用模式--------------------------
enum __attribute__((packed)) RFR_FULL_MODE {
    RFR_FULL_IDLE = 0,
    RFR_FULL_DUPLEX, //全双工模式
    RFR_FULL_EXIT,
}; //对讲机工作模式
//----------------------------------------------
//
//--------------------连接对讲机编码状态--------------------------
enum __attribute__((packed)) RFR_ENC_STATUS {
    RRA_ENC_IDLE = 0,
    RRA_ENC_STANDBY,
    RRA_ENC_WAITING_START_ACK,
    RRA_ENCODING,
};

//--------------------连接对讲机解码状态--------------------------
enum __attribute__((packed)) RFR_DEC_STATUS {
    RRA_DEC_IDLE = 0,
    RRA_DECODING,
    RRA_DEC_WAITING_START_ACK,
};

typedef struct __rfr_mge {
    volatile enum RFR_DEC_STATUS dec_status;
    volatile enum RFR_ENC_STATUS enc_status;
    queue_obj speaker_queue;
    queue_obj listener_queue;
} connect_rf_mge;

extern const audio2rf_send_mge_ops rf_radio_ops;

void rf_connect_set_mode(u8 mode);
u32 rra_send_ack_cmd(queue_obj *p_queue, u8 ack_cmd, u8 ack_data);
int rrapp_idle(rev_fsm_mge *p_recv_ops);    //IDLE应用模式

void rf_radio_connect_app(void);
#endif
