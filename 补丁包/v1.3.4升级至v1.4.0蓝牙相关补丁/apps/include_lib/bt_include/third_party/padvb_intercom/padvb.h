#ifndef __PADVB_API_H__
#define __PADVB_API_H__

#include "my_malloc.h"
#include "jiffies.h"
#include "common.h"
#include "log.h"

#define  OPEN  1
#define  CLOSE 0

#define PADVB_TX_DEBUG_LOG  OPEN
#define PADVB_TX_DEBUG_IO   CLOSE

#define PADVB_RX_DEBUG_LOG  OPEN
#define PADVB_RX_DEBUG_IO   CLOSE

#define PADVB_TX_UNIT_US    CLOSE

#define PADVB_USER_DATA_LEN  160

//必须配置且只能配置一项
#define PADVB_TX_L OPEN
#define PADVB_TX_G CLOSE
#define PADVB_TX_H CLOSE

typedef enum {
    PADVB_EVENT_INIT,
    PADVB_EVENT_UNINIT,
    PADVB_EVENT_OPEN,
    PADVB_EVENT_CLOSE,
    PADVB_EVENT_SYNC_SUCCESS,
    PADVB_EVENT_SYNC_LOST,
    PADVB_EVENT_PUSH_DATA,
    PADVB_EVENT_POP_DATA,
    PADVB_EVENT_PUSH_DATA_TIMEOUT,
} PADVB_EVENT;

typedef struct {
    void (*event_cb)(const PADVB_EVENT event);
} padvb_callback_t;

typedef struct {
    uint8_t            retry_num;       //Number of repetitions of a frame of data
    uint8_t            flow_mode;       //0:timebase 1:count
    uint8_t            phy_mode;        //0:1M 1:code s2
#if PADVB_TX_UNIT_US
    uint32_t           retry_interval;  //unit:us
#endif
    uint16_t           frame_len;       //unit:octet  MAX:160
    uint32_t           frame_interval;  //unit:0.625ms
    uint8_t            adv_name[28];    //name len max 28 octet
    padvb_callback_t   user_cb;
    union {
        struct {
            uint8_t reserve;
        } tx;

        struct {
            uint8_t reserve;
        } rx;
    };
} padvb_user_param_t;

typedef struct {
    void (*padvb_init)(padvb_user_param_t *param);
    void (*padvb_uninit)();
    void (*padvb_open)(void);
    void (*padvb_close)(void);
    void (*padvb_push_data)(uint8_t *data, u16 data_len);
    void (*padvb_pop_data)(uint8_t **data);
    void (*padvb_enter_pair)(void);
    void (*padvb_exit_pair)(void);
} padvb_lib_ops;

extern const padvb_lib_ops padvb_tx_op;
extern const padvb_lib_ops padvb_rx_op;

#endif
