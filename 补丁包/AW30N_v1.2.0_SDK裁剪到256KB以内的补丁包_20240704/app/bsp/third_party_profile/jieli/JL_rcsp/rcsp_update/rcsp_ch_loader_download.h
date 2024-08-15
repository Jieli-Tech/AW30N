#ifndef __JL_RCSP_CH_LOADER_DOWNLOAD_H__
#define __JL_RCSP_CH_LOADER_DOWNLOAD_H__

#include "typedef.h"

void rcsp_update_data_api_register(u32(*data_send_hdl)(void *priv, u32 offset, u16 len), u32(*send_update_status_hdl)(void *priv, u8 state));
void rcsp_update_data_api_unregister(void);
void rcsp_update_handle(u8 state, void *buf, int len);
void rcsp_update_loader_download_init(int update_type, void (*result_cbk)(void *priv, u8 type, u8 cmd));
#endif
