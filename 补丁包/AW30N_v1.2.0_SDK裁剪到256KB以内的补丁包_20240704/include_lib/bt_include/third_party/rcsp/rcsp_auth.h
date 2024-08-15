#ifndef _RCSP_AUTH_H_
#define _RCSP_AUTH_H_

#include "typedef.h"

int rcsp_auth_init(u8 *buf[], u32 buf_len);
void make_rand_num(u8 *buf);

#endif
