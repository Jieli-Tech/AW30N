#include "rcsp_msg.h"
#include "rcsp_bluetooth.h"
#include "common/circular_buf.h"
#include "rcsp_user_update.h"
#include "msg.h"

#define LOG_TAG_CONST      RCSP_MODE
#define LOG_TAG             "[RCSP_MSG]"
#include "log.h"

#if RCSP_BTMATE_EN
static cbuffer_t msg_cbuf;
static u32 msg_pool[128];
void rcsp_msg_init(void)
{
    cbuf_init(&msg_cbuf, msg_pool, sizeof(msg_pool));
}

int rcsp_msg_post(u8 msg, int argc, ...)
{
    u32 param;
    u16 *t_msg = (u16 *)&param;
    CPU_SR_ALLOC();
    va_list argptr;
    OS_ENTER_CRITICAL();
    va_start(argptr, argc);
    if (!cbuf_is_write_able(&msg_cbuf, (argc - 1)*sizeof(int) + MSG_HEADER_BYTE_LEN)) {
        OS_EXIT_CRITICAL();
        return MSG_BUF_NOT_ENOUGH;
    }
    t_msg[0] = (MSG_HEADER_ALL_BIT >> MSG_PARAM_BIT_LEN) & msg;
    t_msg[0] = ((argc) << (MSG_TYPE_BIT_LEN)) | t_msg[0];
    cbuf_write(&msg_cbuf, (void *)&t_msg[0], MSG_HEADER_BYTE_LEN);
    for (u32 i = 0; i < argc; i++) {
        param = va_arg(argptr, int);
        cbuf_write(&msg_cbuf, (void *)&param, sizeof(int));
    }
    va_end(argptr);
    OS_EXIT_CRITICAL();
    rcsp_resume_do();
    return MSG_NO_ERROR;
}

int JL_rcsp_event_handler(RCSP_MSG msg, int argc, int *argv)
{
    int ret = 0;
#if UPDATE_V2_EN
    JL_rcsp_msg_deal(msg, argc, argv);
#endif
    return ret;
}

extern void vPortSuppressTicksAndSleep(u32 usec);
int rcsp_get_msg(int len, int *msg)
{
    u32 param = 0;
    u16 *t_msg = (u16 *)&param;
    CPU_SR_ALLOC();
    OS_ENTER_CRITICAL();
    u32 tlen = cbuf_read(&msg_cbuf, (void *)t_msg, MSG_HEADER_BYTE_LEN);
    if (MSG_HEADER_BYTE_LEN != tlen) {
        OS_EXIT_CRITICAL();
        /* vPortSuppressTicksAndSleep(-2); */
        /* __asm__ volatile("idle"); */
        msg[0] = NO_MSG;
        return MSG_NO_MSG;
    }
    msg[0] = t_msg[0] & (MSG_HEADER_ALL_BIT >> MSG_PARAM_BIT_LEN);
    u32 param_len = param >> MSG_TYPE_BIT_LEN;
    if (param_len > (len - 1)) {
        OS_EXIT_CRITICAL();
        return MSG_BUF_NOT_ENOUGH;
    }
    u32 rlen = cbuf_read(&msg_cbuf, (void *)(msg + 1), param_len * sizeof(int));
    if ((param_len * sizeof(int)) != rlen) {
        OS_EXIT_CRITICAL();
        return MSG_CBUF_ERROR;
    }

    OS_EXIT_CRITICAL();
    return MSG_NO_ERROR;
}

#endif
