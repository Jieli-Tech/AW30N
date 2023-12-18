#ifndef _TESTBOX_UART_UPDATE_H_
#define _TESTBOX_UART_UPDATE_H_
#include "typedef.h"
#include "gpio_hw.h"

// #define DMA_BUF_LEN			32

#define TCFG_UART_UPDATE_PORT		IO_PORTA_00//BD49

#define UART_UPDATE_NUM		 1

#define DMA_BUF_LEN			(512 + 32)

enum {
    UART_UPDATE_START = 0,
    UART_UPDATE_OTA_RECV,
    UART_UPDATE_OTA,
    UART_UPDATE_SUCC,
};

u8 uart_update_get_step(void);
void uart_update_maskrom_start(void *buf, u32 len);
u8 uart_update_ota_loop(u8 *buf, u32 len);


void uart_update_data_deal(u8 *data, u8 len);
void uart_update_data_init();
void uart_update_maskrom_start(void *buf, u32 len);

void testbox_uart_update_init(void);

#endif



