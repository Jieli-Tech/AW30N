#ifndef __RF_BASIC_H__
#define __RF_BASIC_H__
#include "typedef.h"
#include "ump3_encoder.h"
#include "opus_encoder.h"
#include "speex_encoder.h"
#include "sbc_encoder.h"
#include "ima_stream_enc_api.h"
#include "jla_lw_encoder.h"
#include "circular_buf.h"

typedef union {
    u8 enc_ump2[UMP2_ENC_OUTPUT_MAX_SIZE];
    u8 enc_opus[OPUS_ENC_OUTPUT_MAX_SIZE];
    u8 enc_speex[SPEEX_ENC_OUTPUT_MAX_SIZE];
    u8 enc_sbc[SBC_ENC_OUTPUT_MAX_SIZE];
} ENC_STREAM_INPUT_MAX_SIZE;

// #define ENC_OUTPUT_MAX_SIZE             (MAX(UMP2_ENC_OUTPUT_MAX_SIZE, OPUS_ENC_OUTPUT_MAX_SIZE))
#define SENDER_BUF_SIZE                 (sizeof(RF_RADIO_PACKET) + sizeof(ENC_STREAM_INPUT_MAX_SIZE))

typedef enum {
    RADIO_IDLE = 0,
    RADIO_SENDIND,
    RADIO_READY_RECEIVE,
    RADIO_RECEIVING,
    RADIO_NEED_STOP_RECEIVING,
} RADIO_STATUS;

typedef enum {
    AUDIO2RF_START_PACKET   = 0,
    AUDIO2RF_DATA_PACKET    = 1,
    AUDIO2RF_STOP_PACKET    = 2,
    HID2RF_KEY_PACKET    	= 3,
    AUDIO2RF_ACK            = 0x80,
} RADIO_PACKET_TYPE;
#define NEED_INDEX_TYPE     AUDIO2RF_DATA_PACKET + 1
#define PACKET_USE_TOTAL_INDEX 0 //是否统计总包数
#define PACKET_HEAD (0xaa55)

typedef struct _RF_RADIO_PACK {
    u16 header; //0xaa55
    u8 crc8_l;
    u8 type;    //RADIO_PACKET_TYPE
    u16 data_length;
#if PACKET_USE_TOTAL_INDEX
    u32 packet_index;
#endif
    u16 packet_type_cnt;
    u8 data[0];
} _GNU_PACKED_ RF_RADIO_PACKET;

#define RADIO_PACKET_CRC_LEN (sizeof(RF_RADIO_PACKET) - ((u32)&((RF_RADIO_PACKET *)0)->crc8_l + 1))

typedef struct _RF_RADIO_ENC_HEAD {
    u8 enc_type;
    u8 reserved;
    u16 sr;
    u16 br;
} _GNU_PACKED_ RF_RADIO_ENC_HEAD;

typedef enum {
    REV_HEADER_0x55 = 0,
    REV_HEADER_0xaa,
    REV_CRC,
    REV_TYPE,
    REV_LEN_L,
    REV_LEN_H,
#if PACKET_USE_TOTAL_INDEX
    REV_PINDEX_0,
    REV_PINDEX_1,
    REV_PINDEX_2,
    REV_PINDEX_3,
#endif
    REV_SELF_PTCNT_0,
    REV_SELF_PTCNT_1,
    // REV_PTCNT_2,
    // REV_PTCNT_3,
    REV_DATA,
} rev_status;

typedef struct _data_cache_mge {
    u32 offset;
    u8  buf[sizeof(ENC_STREAM_INPUT_MAX_SIZE)];
} data_cache_mge;

typedef struct __ar_unpacket_fsm {
#if PACKET_USE_TOTAL_INDEX
    u32 packet_index;
#endif
    u16 packet_type_cnt;
    rev_status status;
    u16 crc_bk;
    u16 length;
    u16 got_length;
    u8 crc;
    u8 type;
} ar_unpacket_fsm;

typedef struct __rev_fsm_mge {
    data_cache_mge cache;
    cbuffer_t cmd_cbuf;
    void *dec_obj;
    ar_unpacket_fsm fsm;
} rev_fsm_mge;

#define DATA_CACHE_BUF_SIZE (sizeof(((data_cache_mge *)0)->buf))

#define TRANS_EVENT_RECV_NEED_START 0
#define TRANS_EVENT_RECV_NEED_STOP  1
#define TRANS_EVENT_START_ACK_SUCC  2
#define TRANS_EVENT_START_ACK_FAIL  3
#define TRANS_EVENT_MAX             4
extern const u16 trans_event2msg[TRANS_EVENT_MAX];

void set_radio_status(RADIO_STATUS status);
void audio_rf_clr_buf(void);
#endif
