#ifndef  __UART_TYPES_H__
#define  __UART_TYPES_H__

#define     HW_UART_NUM     3

#define CONFIG_ENABLE_UART_OS_SEM  0

enum uart_port {
    UART_NUM_0,
    UART_NUM_1,
    UART_NUM_2,
};

//UART_CON0
#define     UART_TX_PND         (1<<15)
#define     UART_RX_PND         (1<<14)
#define     UART_CLR_TX_PND     (1<<13)
#define     UART_CLR_RX_PND     (1<<12)

#define     UART_OT_PND         (1<<11)
#define     UART_CLR_OT_PND     (1<<10)

#define     UART_RX_FLUSH_OVER   (1<<8)
#define     UART_RX_FLUSH       (1<<7)
#define     UART_RX_DMA         (1<<6)
#define     UART_OT_IE          (1<<5)
#define     UART_DIVS           (1<<4)
#define     UART_RX_IE          (1<<3)
#define     UART_TX_IE          (1<<2)
#define     UART_RX_EN          (1<<1)
#define     UART_TX_EN          (1<<0)

#define     UART_CLR_ALL_PND    (UART_CLR_OT_PND | UART_CLR_RX_PND | UART_CLR_TX_PND)
//UART_CON1
#define     UART_TX_INV         (1<<9)
#define     UART_RX_INV         (1<<8)
#define     UART_RX_DISABLE     (1<<4)

//UART_CON2(parity)
#define     UART_CHK_PND         (1<<8)
#define     UART_CLR_CHK_PND     (1<<7)
#define     UART_CHK_IE          (1<<6)
#define     UART_CHK_EN          (1<<3)
#define     UART_RB8         (1<<2)
#define     UART_TB8         (1<<1) //reserved
#define     UART_M9_EN       (1<<0)
#endif


