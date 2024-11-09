#ifndef __CUSTOM_EVENT_H__
#define __CUSTOM_EVENT_H__

#include "typedef.h"

void clear_custom_event(u32 event, u32 *event_buf, u32 size);
void post_custom_event(u32 event, u32 *event_buf, u32 size);
u32 get_custom_event(u32 event_buf);
#endif
