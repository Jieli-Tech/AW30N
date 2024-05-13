#ifndef	_USB_H_
#define _USB_H_
#include "typedef.h"
// #include "generic/ioctl.h"


#ifndef min
#define min(a,b) ((a)<(b) ? (a) : (b))
#endif
#ifndef USB_DIR_OUT
#define USB_DIR_OUT			0		/* to device */
#endif
#ifndef USB_DIR_IN
#define USB_DIR_IN			0x80		/* to host */
#endif

#define     FUSB_MODE               1
#define     EP0_SETUP_LEN           0x40
#define USB_MAX_HW_EPNUM    5

/* #define ep_regs JL_USB_EP_TypeDef */
typedef struct {
    volatile u32 TXMAXP;
    volatile u32 TXCSR1;
    volatile u32 TXCSR2;
    volatile u32 RXMAXP;
    volatile u32 RXCSR1;
    volatile u32 RXCSR2;
    volatile const u32 RXCOUNT1;
    volatile const u32 RXCOUNT2;
    volatile u32 TXTYPE;
    volatile u32 TXINTERVAL;
    volatile u32 RXTYPE;
    volatile u32 RXINTERVAL;
    u32 RESERVED[0xd0 / 4];
} ep_regs;


//USB_CON0 register
// #define RESERVED        0
#define LOW_SPEED       1
#define USB_NRST        2
#define TM1             3
#define CID             4
#define VBUS            5
// #define RESERVED        6~9
#define SOFIE           10
#define SIEIE           11
#define CLR_SOF         12
#define SOF_PND         13
#define SIE_PND         14
// #define RESERVED        15~17
#define LOWP_MD_        18
#define RST_STL         19
#define EP1_DISABLE     20
#define EP2_DISABLE     21
// #define RESERVED        22
// #define RESERVED        23
#define EP1_RLIM_EN     24
#define EP2_RLIM_EN     25
// #define RESERVED        26
// #define RESERVED        27
#define TX_BLOCK_EN     28
// #define RESERVED        29~31


//USB_CON1 register
#define EP1_MTX_EN      0
#define EP1_MRX_EN      1
#define EP1_MTX_PND_CLR 2
#define EP1_MRX_PND_CLR 3
#define EP1_MTX_PND     4
#define EP1_MRX_PND     5
// #define RESERVED        6~31


//JL_PORTUSB->CON register
#define RCVEN           0
// #define RESERVED        1
#define SR0             2
// #define RESERVED        3
#define PDCHKDP         4
// #define RESERVED        5
#define DIDF            6
#define CHKDPO          7
#define IO_MODE         8
// #define RESERVED        9~10
#define DBG_SEL         11  //2bit
// #define RESERVED        13~31



//USB_SOF_STA_CON reguster
#define FORCE_TXSOF_EN  0
#define SOF_STA0        1
#define SOF_STA1        2
#define SOF_STA2        3
#define USB_SOF_STA_CON_PRD(x)    SFR(JL_USB->SOF_STA_CON,  4, 28,  x)


//USB_TXDLY_CON register
#define CLK_DIS         0
#define CLR_PND         1
#define PND             2
// #define RESERVED        3~15
#define USB_TXDLY_CON_PRD(x)    SFR(JL_USB->TXDLY_CON,  16,  16,  x)

#define USB_EP1_RX_LEN(x)    SFR(JL_USB->EP1_RLEN,  0,  11,  x)
#define USB_EP2_RX_LEN(x)    SFR(JL_USB->EP2_RLEN,  0,  11,  x)

#define USB_EP1_MTX_PRD(x)    SFR(JL_USB->EP1_MTX_PRD,  0, 8,  x)
#define USB_EP1_MTX_NUM(x)    SFR(JL_USB->EP1_MTX_NUM,  0, 8,  x)
#define USB_EP1_MRX_PRD(x)    SFR(JL_USB->EP1_MRX_PRD,  0, 8,  x)
#define USB_EP1_MRX_NUM(x)    SFR(JL_USB->EP1_MRX_NUM,  0, 8,  x)



enum {
    USB0,
};
#define USB_MAX_HW_NUM      1


struct usb_ep_addr_t {
    u32 ep0_addr;
    u32 ep_usage;
    const u8 *ep_taddr[4];
    const u8 *ep_dual_taddr[4];
    u8 *ep_raddr[4];
    u8 *ep_dual_raddr[4];
    u32 ep_tsize[4];
    u32 ep_rsize[4];
} __attribute__((aligned(4)));


typedef u8 usb_dev;

u16 usb_read_sofframe(const usb_dev id);
u32 usb_dev_con0(const usb_dev usb_id);
u32 usb_check_dpo(const usb_dev id);
void usb_sie_enable(const usb_dev usb_id);
void usb_sie_disable(const usb_dev id);
void usb_write_ep_cnt(const usb_dev usb_id, u32 ep, u32 len);
u32 usb_phy_status(const usb_dev id);
u32 usb_g_dev_status(const usb_dev usb_id);
u32 usb_h_dev_status(const usb_dev usb_id);
void usb_set_low_speed(const usb_dev usb_id, u8 flag);
void usb_write_ep0(const usb_dev usb_id, const u8 *ptr, u32 len);
void usb_read_ep0(const usb_dev usb_id, u8 *ptr, u32 len);
void *usb_get_dma_taddr(const usb_dev usb_id, u32 ep);
u32 usb_get_dma_size(const usb_dev usb_id, u32 ep);
void usb_set_dma_tsize(const usb_dev usb_id, u32 ep, u32 size);
void usb_set_dma_rsize(const usb_dev usb_id, u32 ep, u32 size);
void usb_set_dma_taddr(const usb_dev usb_id, u32 ep, const void *ptr);
void *usb_get_dma_raddr(const usb_dev usb_id, u32 ep);
void usb_set_dma_raddr(const usb_dev usb_id, u32 ep, void *ptr);
void usb_set_dma_dual_raddr(const usb_dev usb_id, u32 ep, void *ptr);
void musb_write_index(const usb_dev usb_id, u32 endpoint);
void usb_write_power(const usb_dev usb_id, u32 value);
u32 usb_read_power(const usb_dev usb_id);
u32 usb_read_devctl(const usb_dev usb_id);
void usb_write_devctl(const usb_dev usb_id, u32 value);
u32 usb_read_csr0(const usb_dev usb_id);
void usb_write_csr0(const usb_dev usb_id, u32 csr0);
void usb_ep0_ClrRxPktRdy(const usb_dev usb_id);
void usb_ep0_TxPktEnd(const usb_dev usb_id);
void usb_ep0_RxPktEnd(const usb_dev usb_id);
void usb_ep0_Set_Stall(const usb_dev usb_id);
u32 usb_read_count0(const usb_dev usb_id);
void usb_read_intre(const usb_dev usb_id,
                    u32 *const intr_usbe,
                    u32 *const intr_txe,
                    u32 *const intr_rxe);

void usb_read_intr(const usb_dev usb_id,
                   u32 *const intr_usb,
                   u32 *const intr_tx,
                   u32 *const intr_rx);
void usb_write_intr_usbe(const usb_dev usb_id, u32 intr_usbe);
void usb_set_intr_txe(const usb_dev usb_id, const u32 ep);
void usb_clr_intr_txe(const usb_dev usb_id, const u32 ep);
void usb_set_intr_rxe(const usb_dev usb_id, const u32 ep);
void usb_clr_intr_rxe(const usb_dev usb_id, const u32 ep);
void usb_write_faddr(const usb_dev usb_id, u32 addr);
void usb_write_txcsr(const usb_dev usb_id, const u32 ep, u32 txcsr);
u32 usb_read_txcsr(const usb_dev usb_id, const u32 ep);
void usb_write_rxcsr(const usb_dev usb_id, const u32 ep, u32 rxcsr);
u32 usb_read_rxcsr(const usb_dev usb_id, const u32 ep);
void usb_write_rxmaxp(const usb_dev usb_id, const u32 ep, u32 value);
void usb_write_txmaxp(const usb_dev usb_id, const u32 ep, u32 value);
void usb_write_rxtype(const usb_dev usb_id, const u32 ep, u32 value);
void usb_write_txtype(const usb_dev usb_id, const u32 ep, u32 value);
u32 usb_read_rxcount(const usb_dev usb_id, u32 ep);
u32 usb_g_ep_config(const usb_dev usb_id, const u32 ep, u32 type, u32 ie, u8 *ptr, u32 dma_size);
u32 usb_g_ep_read64byte_fast(const usb_dev usb_id, const u32 ep, u8 *ptr, u32 len);
u32 usb_g_ep_read(const usb_dev usb_id, const u32 ep, u8 *ptr, u32 len, u32 block);
u32 usb_g_ep_write(const usb_dev usb_id, u32 ep, const u8 *ptr, u32 len);
u32 usb_g_ep_config(const usb_dev usb_id, u32 ep, u32 type, u32 ie, u8 *ptr, u32 dma_size);
void usb_g_sie_init(const usb_dev usb_id);
void usb_g_hold(const usb_dev usb_id);
u32 usb_get_ep_num(const usb_dev usb_id, u32 ep_dir, u32 type);
u32 usb_h_ep_config(const usb_dev usb_id, u32 ep, u32 type, u32 ie, u32 interval, u8 *ptr, u32 dma_size);
void usb_mdelay(unsigned int ms);
u32 usb_h_ep_write(const usb_dev usb_id, u8 host_ep, u16 txmaxp, u8 target_ep, const u8 *ptr, u32 len, u32 xfer);
int usb_h_ep_write_async(const usb_dev id, u8 host_ep, u16 txmaxp, u8 target_ep, const u8 *ptr, u32 len, u32 xfer, u32 kstart);
u32 usb_h_ep_read(const usb_dev usb_id, u8 host_ep, u16 rxmaxp, u8 target_ep, u8 *ptr, u32 len, u32 xfer);
int usb_h_ep_read_async(const usb_dev id, u8 host_ep, u8 target_ep, u8 *ptr, u32 len, u32 xfer, u32 kstart);
void usb_h_sie_init(const usb_dev usb_id);
void usb_h_sie_close(const usb_dev usb_id);
u32 usb_h_chirp_and_reset(const usb_dev id, u32 reset_delay, u32 timeout);
void usb_h_sie_reset(const usb_dev usb_id);
void usb_hotplug_disable(const usb_dev usb_id);
void usb_hotplug_enable(const usb_dev usb_id, u32 mode);
void usb_pdchkdp_disable(const usb_dev usb_id);
void usb_pdchkdp_enable(const usb_dev usb_id);
void usb_sie_close(const usb_dev usb_id);
void usb_sie_close_all(void);
void usb_var_init(const usb_dev usb_id, void *ptr);
void usb_var_release(const usb_dev usb_id);
void usb_enable_ep(const usb_dev usb_id, u32 eps);
void usb_disable_ep(const usb_dev usb_id, u32 eps);
u32 usb_get_ep_status(const usb_dev usb_id, u32 epx);

void usb_sofie_enable(const usb_dev id);
void usb_sofie_disable(const usb_dev id);
void usb_sof_clr_pnd(const usb_dev id);
void usb_ep0_Set_ignore(const usb_dev id, u32 addr);
void usb_recover_io_status(const usb_dev id);

//usb_suspend_resume
#include "asm/power_interface.h"
void usb_slave_phy_suspend(const usb_dev usb_id);
void usb_slave_phy_resume(const usb_dev usb_id);
void usb_slave_dp_wakeup_enable(const usb_dev usb_id);
void usb_slave_dp_wakeup_disable(const usb_dev usb_id);
void usb_slave_dp_wakeup_init(const usb_dev usb_id, void (*dp_wkup_cb)(P33_IO_WKUP_EDGE edge));
#endif
