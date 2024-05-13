#ifndef __RF_RADIO_RS_H__
#define __RF_RADIO_RS_H__

#include "typedef.h"
#include "msg.h"

#include "circular_buf.h"
#include "tick_timer_driver.h"
//
#include "audio_dac_api.h"
//
#include "encoder_mge.h"
#include "decoder_api.h"
//
#include "audio_rf_mge.h"
#include "audio2rf_send.h"
#include "rf2audio_recv.h"
//
#include "rf_send_queue.h"
#include "trans_unpacket.h"
//
#include "bt_ble.h"
#include "vble_complete.h"
#include "vble_simple.h"

#define APP_SOFTOFF_CNT_TIME_10MS  3000 //3000 * 10ms

#if APP_SOFTOFF_CNT_TIME_10MS
#define app_softoff_time_reset(n) (n = maskrom_get_jiffies() + APP_SOFTOFF_CNT_TIME_10MS)
#else
#define app_softoff_time_reset(n)
#endif
//interval >= 4000配置为私有连接参数,interval=4000,实际连接间隔为4000us
//interval <  600 配置为标准连接参数,interval=30,实际连接间隔为30*1.25ms=37.5ms
#define RADIO_WORKING_INTERVAL      4000
#define RADIO_STANDBY_INTERVAL      50000

typedef enum {
    RRA_IDLE_MODE = 0,
    RRA_SEND_MODE,
    RRA_RECIVE_MODE,
    RRA_FULL_DUPLEX, //全双工模式
    RRA_EXIT,
} RRA_MODE; //对讲机工作模式

typedef enum {
    RRA_ENC_IDLE = 0,
    RRA_ENCODING,
    RRA_ENC_WAITING_START_ACK,
} ENC_STATUS; //编码状态

typedef enum {
    RRA_DEC_IDLE = 0,
    RRA_DECODING,
    RRA_DEC_WAITING_START_ACK,
} DEC_STATUS; //解码状态

typedef struct __radio_mge_struct {
    rev_fsm_mge packet_recv;
    sound_stream_obj stream;
    cbuffer_t dec_icbuf;
    cbuffer_t ack_cbuf;
    cbuffer_t cmd_cbuf;
    volatile enc_obj *enc_obj;
    u32 app_softoff_jif_cnt;
    u32 app_standby_jif_cnt;
    // volatile u8 rra_status;
    volatile u8 rra_enc_status;
    volatile u8 rra_dec_status;
    volatile u8 rra_mode;
    volatile u8 rra_event_occur;
} radio_mge_struct;
//
extern radio_mge_struct radio_mge;
extern u8 rra_packet[16];

void rra_in_idle(void);
u32 rra_send_ack_cmd(u8 ack_cmd, u8 ack_data);
void rrapp_send_queue_init(void);



/*********************  ****************************/
enc_obj *rra_encode_init(void);
void rra_encode_start(void);
void rra_encode_stop(ENC_STOP_WAIT wait);
bool rrapp_sending(int active_msg);
void fd_radio_encode_idle();


/*********************  ****************************/
dec_obj *rra_decode_start(RF_RADIO_ENC_HEAD *p_enc_head);
void rra_decode_stop(void);
u8 rrapp_receiving(int active_msg);
void fd_radio_decode_idle();
void fd_radio_encode_idle();

u8 fd_rrapp_loop(int active_msg);

#endif

