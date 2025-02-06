
#ifndef __PADV_RADIO_H__
#define __PADV_RADIO_H__

//--------------------padv radio--------------------------
typedef enum __attribute__((packed))
{
    PADV_RX_IDLE = 0,
    PADV_RX_READY,
}
PADV_RX_STATUS;

typedef enum __attribute__((packed))
{
    PADV_TX_IDLE = 0,
    PADV_TX_READY,
}
PADV_TX_STATUS; //RX状态
//----------------------------------------------

typedef struct __padv_mge {
    volatile PADV_RX_STATUS padv_rx_status;
    volatile PADV_TX_STATUS padv_tx_status;
    queue_obj speaker_queue;
    queue_obj listener_queue;
} padv_mge;


void rf_radio_padv_app(void);

#endif
