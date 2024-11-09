#pragma bss_seg(".cutm_evt.data.bss")
#pragma data_seg(".cutm_evt.data")
#pragma const_seg(".cutm_evt.text.const")
#pragma code_seg(".cutm_evt.text")
#pragma str_literal_override(".cutm_evt.text.const")

#include "custom_event.h"
#include "common.h"
#include "config.h"
#include "msg.h"

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[cutm_evt]"
#include "log.h"

void clear_custom_event(u32 event, u32 *event_buf, u32 size)
{
    if (event >= size) {
        return;
    }
    OS_ENTER_CRITICAL();
    *event_buf &= ~BIT(event % 32);
    OS_EXIT_CRITICAL();
}
void post_custom_event(u32 event, u32 *event_buf, u32 size)
{
    if (event >= size) {
        return;
    }
    /* log_info(">>> evenr post : 0x%x\n", event); */
    OS_ENTER_CRITICAL();
    *event_buf |= BIT(event % 32);
    OS_EXIT_CRITICAL();
}
u32 get_custom_event(u32 event_buf)
{
    OS_ENTER_CRITICAL();
    u32 event_cls;
    __asm__ volatile("%0 = clz(%1)":"=r"(event_cls):"r"(event_buf));
    if (event_cls != 32) {
        OS_EXIT_CRITICAL();
        /* log_info(" has event 0x%x\n", (31 - event_cls)); */
        return (31 - event_cls);
    }
    OS_EXIT_CRITICAL();
    return NO_EVENT;
}
