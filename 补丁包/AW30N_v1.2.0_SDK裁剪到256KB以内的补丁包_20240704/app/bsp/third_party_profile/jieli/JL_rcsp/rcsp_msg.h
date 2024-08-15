#ifndef __RCSP_MSG_H__
#define __RCSP_MSG_H__

#include "typedef.h"

typedef enum __RCSP_MSG {
    // RCSP_MSG_UPDATE_EQ = 0,
    // RCSP_MSG_SET_FMTX_POINT,
    // RCSP_MSG_BS_END,
    // RCSP_MSG_BT_SCAN,
    RCSP_MSG_NORMAL_NORMAL_PROCESS,
    RCSP_MSG_END,
} RCSP_MSG;

int rcsp_msg_post(u8 msg, int argc, ...);
void rcsp_msg_init(void);
int JL_rcsp_event_handler(RCSP_MSG msg, int argc, int *argv);
#endif
