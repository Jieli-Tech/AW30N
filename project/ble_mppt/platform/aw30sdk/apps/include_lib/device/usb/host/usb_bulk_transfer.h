#ifndef __USB_BULK_TRANSFER_H__
#define __USB_BULK_TRANSFER_H__
#include "typedef.h"


s32 usb_bulk_only_receive_async(struct device *device, u8 host_ep, u16 rxmaxp, u8 target_ep, u8 *pBuf, u32 len);
s32 usb_bulk_only_receive(struct device *device, u8 host_ep, u16 rxmaxp, u8 ep, u8 *pBuf, u32 len);
s32 usb_bulk_only_send_async(struct device *device, u8 host_ep, u16 txmaxp, u8 target_ep, const u8 *pBuf, u32 len);
s32 usb_bulk_only_send(struct device *device, u8 host_ep, u16 txmaxp, u8 ep, const u8 *pBuf, u32 len);

s32 usb_bulk_receive_async_no_wait(struct device *device, u8 host_ep, u16 rxmaxp, u8 target_ep, u8 *pBuf, u32 len);
#endif
