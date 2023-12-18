#ifndef __RF_BASIC_H__
#define __RF_BASIC_H__
#include "typedef.h"
#include "ump3_encoder.h"
#include "opus_encoder.h"


#define ENC_OUTPUT_MAX_SIZE             (MAX(UMP2_ENC_OUTPUT_MAX_SIZE, OPUS_ENC_OUTPUT_MAX_SIZE))
#define SENDER_BUF_SIZE                 (sizeof(RF_RADIO_PACKET) + ENC_OUTPUT_MAX_SIZE)

typedef enum {
    RADIO_IDLE = 0,
    RADIO_SENDIND,
    RADIO_RECEIVING,
} RADIO_STATUS;

typedef enum {
    AUDIO2RF_START_PACKET   = 0,
    AUDIO2RF_DATA_PACKET    = 1,
    AUDIO2RF_STOP_PACKET    = 2,
    HID2RF_KEY_PACKET    	= 3,
} RADIO_PACKET_TYPE;

#define PACKET_HEAD (0xaa55)

typedef struct _RF_RADIO_PACK {
    u16 header; //0xaa55
    u8 crc8_l;
    u8 type;    //RADIO_PACKET_TYPE
    u16 data_length;
    u32 packet_index;
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
    REV_PINDEX_0,
    REV_PINDEX_1,
    REV_PINDEX_2,
    REV_PINDEX_3,
    REV_DATA,
} rev_status;

typedef struct __rev_fsm_mge {
    u32 packet_index;
    rev_status status;
    u16 crc_bk;
    u16 length;
    u16 got_length;
    u8 crc;
    u8 type;
} rev_fsm_mge;

void set_radio_status(RADIO_STATUS status);
void audio_rf_clr_buf(void);
#endif
