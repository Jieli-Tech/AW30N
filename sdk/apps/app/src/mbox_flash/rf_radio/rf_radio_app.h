#ifndef __RF_RADIO_APP_H__
#define __RF_RADIO_APP_H__

#include "typedef.h"
#include "app.h"

#if ENCODER_UMP3_EN
#define RA_ENC_SR      32000
#define RA_ENC_TYPE    FORMAT_UMP3
#define RA_ENC_FUNC    ump3_encode_api
#elif ENCODER_OPUS_EN
#define RA_ENC_SR      16000
#define RA_ENC_TYPE    FORMAT_OPUS
#define RA_ENC_FUNC    opus_encode_api
#elif ENCODER_SBC_EN
#define RA_ENC_SR      16000
#define RA_ENC_TYPE    FORMAT_SBC
#define RA_ENC_FUNC    sbc_encode_api
#elif ENCODER_JLA_LW_EN
#define RA_ENC_SR      32000
#define RA_ENC_TYPE    FORMAT_JLA_LW
#define RA_ENC_FUNC    jla_lw_encode_api
#else
#define RA_ENC_SR      0
#define RA_ENC_TYPE    -1
#define RA_ENC_FUNC    NULL
#endif


extern void bt_init_api(void);
extern void app_ble_recv_callback_register(int (*callback_func)(u8 *, u16));
extern u16 rf_radio_key_msg_filter(u8 key_status, u8 key_num, u8 key_type);
extern bool rf_recv_data_check(void *rf_packet, u16 packet_len);
void rf_radio_app(void);

#endif

