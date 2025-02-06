#pragma bss_seg(".rf_queue.data.bss")
#pragma data_seg(".rf_queue.data")
#pragma const_seg(".rf_queue.text.const")
#pragma code_seg(".rf_queue.text")
#pragma str_literal_override(".rf_queue.text.const")

#include "rf_send_queue.h"
#include "crc16.h"
#include "errno-base.h"
#include "circular_buf.h"
#include "audio_rf_mge.h"
#include "trans_packet.h"
#include "encoder_mge.h"
#include "msg.h"
#include "jiffies.h"

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[rf_isr]"
#include "log.h"

/* #define rf_send_mutex_pend()  bit_clr_ie(IRQ_SOFT1_IDX) */
/* #define rf_send_mutex_post()  bit_set_ie(IRQ_SOFT1_IDX) */

#if RF_SENDER_USE_QUEUE



/* static queue_obj g_queue_obj AT(.ar_trans_data); */

#define D_QUEUE_CBUF            obj->cbuf
#define multiple_and(m,bits)    ((bits) == (m & (bits)))
#define is_queue_working(obj)   multiple_and(obj->qs_flag, D_QS_ENABLE)
#define is_queue_can_in(obj)    multiple_and(obj->qs_flag, D_QS_ENABLE | D_QS_CAN_IN)
#define is_queue_can_out(obj)   multiple_and(obj->qs_flag, D_QS_ENABLE | D_QS_CAN_OUT)
#define is_queue_isr_en(obj)    multiple_and(obj->qs_flag, D_QS_ENABLE | D_QS_ISR)



#define IRQ_RF_QUEUE_IDX         IRQ_SOFT2_IDX
#define clr_rf_queue_isr_pend()  bit_clr_swi(IRQ_RF_QUEUE_IDX - IRQ_SOFT0_IDX)
/* #define kick_rf_queue_isr()      bit_set_swi(IRQ_RF_QUEUE_IDX - IRQ_SOFT0_IDX) */

#define QUEUE_ISR_MAX 2
typedef struct _queue_isr_obj {
    queue_obj *p_queue[QUEUE_ISR_MAX];
    u8 enable;
    u8 b_kick;
} queue_isr_obj;

static queue_isr_obj g_queue_isr_obj AT(.ar_trans_data);

void kick_rf_queue_isr(queue_obj *obj)
{
    if (is_queue_isr_en(obj)) {
        for (u32 i = 0; i < QUEUE_ISR_MAX; i++) {
            /* log_info("%d, obj 0x%x, p_queue 0x%x\n", i, obj, g_queue_isr_obj.p_queue[i]); */
            if (obj == g_queue_isr_obj.p_queue[i]) {
                g_queue_isr_obj.b_kick |= BIT(i);
                /* log_info("k %d\n", i); */
            }
        }
        /* log_info("end\n"); */
        /* log_char('K'); */
        bit_set_swi(IRQ_RF_QUEUE_IDX - IRQ_SOFT0_IDX);
    }
}
bool regist_rf_queue_isr(queue_obj *obj)
{
    for (u32 i = 0; i < QUEUE_ISR_MAX; i++) {
        if (0 == (g_queue_isr_obj.enable & BIT(i))) {
            g_queue_isr_obj.p_queue[i] = obj;
            g_queue_isr_obj.b_kick &= ~BIT(i);
            g_queue_isr_obj.enable |= BIT(i);
            /* log_info("obj 0x%x\n", obj); */
            return true;
        }
    }
    return false;
}

bool unregist_rf_queue_isr(queue_obj *obj)
{
    for (u32 i = 0; i < QUEUE_ISR_MAX; i++) {
        if (obj == g_queue_isr_obj.p_queue[i]) {
            g_queue_isr_obj.enable &= ~BIT(i);
            g_queue_isr_obj.p_queue[i] = NULL;
            /* log_info("unregist bit%d\n", i); */
            return true;
        }
    }
    return false;
}

static bool is_queue_isr_empty()
{
    for (u32 i = 0; i < QUEUE_ISR_MAX; i++) {
        if (g_queue_isr_obj.enable & BIT(i)) {
            return false;
        }
    }
    return true;

}

static void rf_send_queue_isr_phy(queue_obj *obj)
{

    if (!is_queue_can_out(obj)) {
        return;
    }
    audio2rf_send_mge_ops *p_send_ops = obj->p_send_ops;
    if (NULL == p_send_ops) {
        log_char('A');
        return;
    }
    if (p_send_ops->check_status() == 0) {
        /* rf无法正常通信 */
        log_char('B');
        return;
    }
    void *rptr;
    u32 wlen, can_send_len, data_len, tlen, len;
    int err = 0;

    can_send_len = p_send_ops->get_valid_len();
    data_len = cbuf_get_data_size(D_QUEUE_CBUF);
    while ((can_send_len >= 10) && (data_len != 0)) {
        rptr = cbuf_read_alloc(D_QUEUE_CBUF, &len);
        if (0 == rptr) {
            log_char('C');
            break;
        }

        tlen = can_send_len > len ? len : can_send_len;

        err = p_send_ops->send(rptr, tlen);
        /* log_info("S:%d 0x%x %d", test_cnt++, rptr, tlen); */
        if (err) {
            log_error("Serr:0x%x", err);
            /* return E_AU2RF_SNED_DATA_ERR; */
        }
        cbuf_read_updata(D_QUEUE_CBUF, tlen);
        can_send_len = p_send_ops->get_valid_len();
        data_len = cbuf_get_data_size(D_QUEUE_CBUF);
    }
}
SET(interrupt(""))
void rf_send_queue_isr(void)
{
    clr_rf_queue_isr_pend();

    for (u32 i = 0; i < QUEUE_ISR_MAX; i++) {
        if (0 == (g_queue_isr_obj.enable & BIT(i))) {
            /* log_error("no_enable %d\n", i); */
            continue;
        }
        if (0 == (g_queue_isr_obj.b_kick & BIT(i))) {
            /* log_error("no_kick %d\n", i); */
            continue;
        }
        queue_obj *obj = g_queue_isr_obj.p_queue[i];
        /* log_info("isr %d\n", i); */
        rf_send_queue_isr_phy(obj);
        g_queue_isr_obj.b_kick &= ~BIT(i);
    }

}

u32 rf_send_push2queue(queue_obj *obj, u8 *packet_data, u16 packet_len)
{
    if (NULL == obj) {
        return E_AU2RF_OBJ_NULL;
    }
    if (!is_queue_can_in(obj)) {
        return E_AU2RF_CAN_NOT_IN;
    }
    u32 wlen = cbuf_get_available_space(D_QUEUE_CBUF);
    u32 res = 0;
    if (wlen >= packet_len) {
        local_irq_disable();
        u32 w_aval_len = cbuf_get_end_addr(D_QUEUE_CBUF) - cbuf_get_writeptr(D_QUEUE_CBUF);
        if ((w_aval_len < packet_len) && (0 == cbuf_get_data_size(D_QUEUE_CBUF))) {
            cbuf_clear(D_QUEUE_CBUF);
        }
        local_irq_enable();
        cbuf_write(D_QUEUE_CBUF, packet_data, packet_len);
    } else {
        putchar('E');
        /* log_error("send_cache full!\n"); */
        res = E_AU2RF_SNED_QUEUE_FULL;
    }
    kick_rf_queue_isr(obj);
    return res;
}

static u32 get_queue_data_size(queue_obj *obj)
{
    if (!is_queue_working(obj)) {
        return 0;
    }

    if (NULL == D_QUEUE_CBUF) {
        return 0;
    }
    return cbuf_get_data_size(D_QUEUE_CBUF);
}


u32 rf_queue_init(queue_obj *obj, audio2rf_send_mge_ops *ops, void *p_cbuf)
{
    u32 res = 0;
    if (NULL == obj) {
        return E_RFQUE_OBJ_NULL;
    }
    obj->qs_flag  &= ~(D_QS_CAN_IN | D_QS_CAN_OUT);

    if (NULL != ops) {
        obj->p_send_ops = ops;
        if (is_queue_isr_empty()) {
            HWI_Install(IRQ_RF_QUEUE_IDX, (u32)rf_send_queue_isr,  IRQ_RF_QUEUE_IP);
        }
        obj->qs_flag  |= D_QS_ISR;
        if (!regist_rf_queue_isr(obj)) {
            res = E_RFQUE_CHN_FULL;
        }
    }
    /* else  */
    /* { */
    /* res = E_RFQUE_OPS_NULL; */
    /* } */

    D_QUEUE_CBUF = p_cbuf;
    obj->qs_flag  |= D_QS_ENABLE | D_QS_CAN_IN | D_QS_CAN_OUT;
    return 0;
}
void rf_queue_uninit(queue_obj *obj, IS_WAIT need_wait)
{
    obj->qs_flag  &= ~D_QS_CAN_IN;
    if (NEED_WAIT == need_wait) {
        u32 retry = 50;
        while ((0 != get_queue_data_size(obj)) && (0 != retry)) {
            /* 设置的广播间隔可能较长 */
            kick_rf_queue_isr(obj);
            delay_10ms(2);
            retry--;
        }
    }
    obj->qs_flag  &= ~D_QS_CAN_OUT;
    if (obj->qs_flag & D_QS_ISR) {
        unregist_rf_queue_isr(obj);
        if (is_queue_isr_empty()) {
            HWI_Uninstall(IRQ_RF_QUEUE_IDX);
        }
        obj->p_send_ops = NULL;
        obj->qs_flag &= ~D_QS_ISR;
    }
    D_QUEUE_CBUF = NULL;
    obj->qs_flag  &= ~D_QS_ENABLE;
    /* log_info("uninit obj->qs_flag 0x%x\n", obj->qs_flag); */
}

u32 read_data_from_queue(queue_obj *obj, u8 *buf, u32 len)
{
    /* if(0 == (obj->qs_flag & D_QS_ENABLE)) */

    if (NULL == obj) {
        return 0;
    }

    if (NULL == D_QUEUE_CBUF) {
        return 0;
    }
    if (!is_queue_can_out(obj)) {
        return 0;
    }
    void *rptr;
    u32 olen, data_len, rlen;
    rlen = len; //剩余的读取长度
    memset(buf, 0, len);
    /* putchar('z'); */
    data_len = cbuf_get_data_size(D_QUEUE_CBUF);
    /* log_info("dlen1 %d\n", data_len); */
    /* u32 rcnt = 0; */
    u32 all_got_len = 0;
    while ((rlen != 0) && (data_len != 0)) {
        /* log_info("%d, %d, %d\n", rlen, data_len, rcnt++); */
        rptr = cbuf_read_alloc(D_QUEUE_CBUF, &olen);
        if (0 == rptr) {
            log_char('C');
            break;
        }
        u32 glen = olen > rlen ? rlen : olen;
        /* log_info("glen %d\n", glen); */
        memcpy(buf, rptr, glen);
        rlen -= glen;
        cbuf_read_updata(D_QUEUE_CBUF, glen);
        data_len = cbuf_get_data_size(D_QUEUE_CBUF);
        buf += glen;
        all_got_len += glen;
    }
    data_len = cbuf_get_data_size(D_QUEUE_CBUF);
    /* log_info("dlen2 %d\n", data_len); */
    return all_got_len;

}

#endif
