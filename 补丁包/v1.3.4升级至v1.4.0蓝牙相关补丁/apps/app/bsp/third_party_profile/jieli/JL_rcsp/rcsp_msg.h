#ifndef __RCSP_MSG_H__
#define __RCSP_MSG_H__

#include "typedef.h"

int rcsp_msg_post(u8 msg, int argc, ...);
void rcsp_msg_init(void);
#endif
