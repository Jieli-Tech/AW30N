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

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[rf_isr]"
#include "log.h"

/* #define rf_send_mutex_pend()  bit_clr_ie(IRQ_SOFT1_IDX) */
/* #define rf_send_mutex_post()  bit_set_ie(IRQ_SOFT1_IDX) */

#if RF_SENDER_USE_QUEUE
cbuffer_t *p_queue_cbuf AT(.ar_trans_data);
/* u8 send_queue_buf[2 * SENDER_BUF_SIZE] AT(.ar_trans_data); */


#define IRQ_RF_QUEUE_IDX         IRQ_SOFT2_IDX
#define clr_rf_queue_isr_pend()  bit_clr_swi(IRQ_RF_QUEUE_IDX - IRQ_SOFT0_IDX)
/* #define kick_rf_queue_isr()      bit_set_swi(IRQ_RF_QUEUE_IDX - IRQ_SOFT0_IDX) */

void kick_rf_queue_isr(void)
{
    bit_set_swi(IRQ_RF_QUEUE_IDX - IRQ_SOFT0_IDX);
}

SET(interrupt(""))
void rf_send_queue_isr(void)
{
    clr_rf_queue_isr_pend();

    audio2rf_send_mge_ops *p_send_ops = g_send_ops;
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
    data_len = cbuf_get_data_size(p_queue_cbuf);

    /* u32 test_cnt = 0; */
    while ((can_send_len >= 10) && (data_len != 0)) {
        rptr = cbuf_read_alloc(p_queue_cbuf, &len);
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
        cbuf_read_updata(p_queue_cbuf, tlen);
        can_send_len = p_send_ops->get_valid_len();
        data_len = cbuf_get_data_size(p_queue_cbuf);
    }
}

/* bool rf_send_push2queue(RADIO_PACKET_TYPE type, u8 *data, u16 data_len, u8 *packet_data) */
u32 rf_send_push2queue(u8 *packet_data, u16 packet_len)
{
    // u16 packet_len = ar_trans_pack(type, data, data_len, packet_data);
    if (NULL == p_queue_cbuf) {
        return E_AU2RF_OBJ_NULL;
    }
    u32 wlen = cbuf_get_available_space(p_queue_cbuf);
    u32 res = 0;
    if (wlen >= packet_len) {
        u32 w_aval_len = cbuf_get_end_addr(p_queue_cbuf) - cbuf_get_writeptr(p_queue_cbuf);
        if ((w_aval_len < packet_len) && (0 == cbuf_get_data_size(p_queue_cbuf))) {
            cbuf_clear(p_queue_cbuf);
        }
        cbuf_write(p_queue_cbuf, packet_data, packet_len);
    } else {
        log_error("send_cache full!\n");
        res = E_AU2RF_SNED_QUEUE_FULL;
    }
    kick_rf_queue_isr();
    return res;
}

void rf_send_soft_isr_init(void *p_cbuf)
{
    p_queue_cbuf = p_cbuf;
    /* cbuf_init(p_queue_cbuf, &send_queue_buf[0], sizeof(send_queue_buf)); */
    HWI_Install(IRQ_RF_QUEUE_IDX, (u32)rf_send_queue_isr,  IRQ_RF_QUEUE_IP) ;
}
#endif
