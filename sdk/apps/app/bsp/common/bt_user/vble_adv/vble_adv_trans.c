#include "vble_adv.h"

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[vble_adv_trans]"
#include "log.h"

padvb_user_param_t u_pa_rx AT(.rf_radio_padv);
padvb_user_param_t u_pa_tx AT(.rf_radio_padv);

u8 g_adv_data_buf[SEND_FRAME_LEN] AT(.rf_radio_padv);

struct PADV_PACKET_HEADER {
    /* u8 id; */
    u16 index;
};
struct PADV_PACKET_HEADER padv_header AT(.rf_radio_padv);

queue2vble_tx_send_mge_ops *g_tx_ops;

void queue2vble_tx_send_register(void *ops)
{
    local_irq_disable();
    g_tx_ops = ops;
    local_irq_enable();
}


void vble_padv_radio_tx_callback(const PADVB_EVENT event)
{
    /* putchar('t'); */
    u32 evt = (u32)event;
    switch (evt) {
    case PADVB_EVENT_INIT:
        log_info("PADVB_EVENT_INIT\n");
        break;
    case PADVB_EVENT_UNINIT:
        log_info("PADVB_EVENT_UNINIT\n");
        break;
    case PADVB_EVENT_OPEN:
        log_info("PADVB_EVENT_OPEN\n");
        memset((void *)&padv_header, 0, sizeof(struct PADV_PACKET_HEADER));
        /* while(0 == padv_header.id) { */
        /* padv_header.id = jiffies + JL_RAND->R64L; */
        /* } */
        /* log_info("tx id is %d\n", padv_header.id); */
        memset(g_adv_data_buf, 0, sizeof(g_adv_data_buf));
        break;
    case PADVB_EVENT_PUSH_DATA_TIMEOUT:
        log_info("PADVB_EVENT_PUSH_DATA_TIMEOUT\n");
        break;
    case PADVB_EVENT_CLOSE:
        log_info("PADVB_EVENT_CLOSE\n");
        break;
    case PADVB_EVENT_PUSH_DATA:
        /* log_info("PADVB_EVENT_PUSH_DATA\n"); */
        if (NULL == g_tx_ops) {
            break;
        }
        /* putchar('p'); */
        memcpy(&g_adv_data_buf[0], &padv_header, sizeof(struct PADV_PACKET_HEADER));
        u32 rlen = u_pa_tx.frame_len - sizeof(struct PADV_PACKET_HEADER);
        u8 *data = g_adv_data_buf + sizeof(struct PADV_PACKET_HEADER);
        queue_obj *obj = g_tx_ops->get_send_queue_obj();
        u32 send_data_len = g_tx_ops->read_data(obj, data, rlen);
        if (send_data_len != 0) {
            /* log_info("ogdlen %d\n", send_data_len); */
            padv_header.index++;
            vble_adv_tx_push_data(g_adv_data_buf, u_pa_tx.frame_len);
        }
        /* log_info_hexdump((u8 *)g_adv_data_buf, 16); */

        break;
    default:
        break;
    }
}

typedef struct _vble_adv_rx_mge {
    void *priv;
    /* void (*sync_success_callback)(); */
    void (*sync_lost_callback)(void);
    void (*pop_data_callback)(void *, u8 *, u16);
} vble_adv_rx_mge;
vble_adv_rx_mge vble_rx_cb_info AT(.rf_radio_padv);
/* int vble_adv_rx_cb_register(void *priv, void *sync_success_callback_fun, void *sync_lost_callback_fun, void *pop_data_callback_fun) */
int vble_adv_rx_cb_register(void *priv, void *sync_lost_callback_fun, void *pop_data_callback_fun)
{
    local_irq_disable();
    vble_rx_cb_info.priv = priv;
    /* vble_rx_cb_info.sync_success_callback = sync_success_callback_fun; */
    vble_rx_cb_info.sync_lost_callback = sync_lost_callback_fun;
    vble_rx_cb_info.pop_data_callback = pop_data_callback_fun;
    local_irq_enable();
    return 0;
}
void vble_padv_radio_rx_callback(const PADVB_EVENT event)
{
    u32 evt = (u32)event;
    u8 *rxdata = NULL;
    switch (evt) {
    case PADVB_EVENT_INIT:
        log_info("PADVB_EVENT_INIT\n");
        break;
    case PADVB_EVENT_UNINIT:
        log_info("PADVB_EVENT_UNINIT\n");
        break;
    case PADVB_EVENT_OPEN:
        log_info("PADVB_EVENT_OPEN\n");
        memset((void *)&padv_header, 0, sizeof(struct PADV_PACKET_HEADER));
        /* memset(g_adv_data_buf, 0, sizeof(g_adv_data_buf)); */
        break;
    case PADVB_EVENT_CLOSE:
        log_info("PADVB_EVENT_CLOSE\n");
        break;
    case PADVB_EVENT_SYNC_SUCCESS:
        log_info("PADVB_EVENT_SYNC_SUCCESS\n");
#if 0
        /* 使用双方约定好的编码参数做解码 */
        /* 该方式在同步后可以立刻开启解码，但是不够灵活*/
        if (NULL != vble_rx_cb_info.sync_success_callback) {
            vble_rx_cb_info.sync_success_callback();
        }
#endif
        break;
    case PADVB_EVENT_SYNC_LOST:
        log_info("PADVB_EVENT_SYNC_LOST\n");
        memset((void *)&padv_header, 0, sizeof(struct PADV_PACKET_HEADER));
        /* 发命令关闭解码 */
        if (NULL != vble_rx_cb_info.sync_lost_callback) {
            vble_rx_cb_info.sync_lost_callback();
        }
        break;
    case PADVB_EVENT_POP_DATA:
        /* log_info("PADVB_EVENT_POP_DATA\n"); */
        vble_adv_rx_pop_data(&rxdata);
        if (rxdata == NULL) {
            /* log_info("lost packet\n"); */
            putchar('.');
            break;
        }
        /* log_info_hexdump(rxdata, 16); */
        struct PADV_PACKET_HEADER tmp_padv_header;
        memcpy((void *)&tmp_padv_header, rxdata, sizeof(struct PADV_PACKET_HEADER));
        u32 wlen = u_pa_rx.frame_len - sizeof(struct PADV_PACKET_HEADER);
        u8 *wdata = rxdata + sizeof(struct PADV_PACKET_HEADER);
        /* log_info_hexdump(rxdata, 16); */
        /* log_info_hexdump((u8 *)g_adv_data_buf, 16); */
        /* printf("%d, %d\n", tmp_padv_header.id, padv_header.id); */
        /* if ((tmp_padv_header.id != padv_header.id) && (padv_header.id != 0)) { */
        /* log_error("id err %d, %d\n", tmp_padv_header.id, padv_header.id); */
        /* break; */
        /* } */
        if (tmp_padv_header.index != padv_header.index) {
            /* putchar('u'); */
            if ((NULL != vble_rx_cb_info.priv) && (NULL != vble_rx_cb_info.pop_data_callback)) {
                void *priv = vble_rx_cb_info.priv;
                vble_rx_cb_info.pop_data_callback(priv, wdata, wlen);
            }
            /* padv_header.id = tmp_padv_header.id; */
            padv_header.index = tmp_padv_header.index;
        }
        break;
    default:
        break;
    }
}



void vble_padv_param_init(void)
{
    memset(&u_pa_rx, 0, sizeof(padvb_user_param_t));
    memset(&u_pa_tx, 0, sizeof(padvb_user_param_t));

    memcpy(u_pa_rx.adv_name, "jla_padva_aw30", 14);
    u_pa_rx.frame_len = SEND_FRAME_LEN; //130
    /* u_pa_rx.frame_interval = 20; //12.5ms */
    u_pa_rx.frame_interval = 32; //20ms
    u_pa_rx.phy_mode = 0;
    u_pa_rx.retry_num = 8;
    u_pa_rx.flow_mode = 0;
    /* u_pa_rx.retry_interval = 1250; */
    u_pa_rx.user_cb.event_cb = vble_padv_radio_rx_callback;

    memcpy(u_pa_tx.adv_name, "jla_padva_aw30", 14);
    u_pa_tx.frame_len = SEND_FRAME_LEN; //130
    /* u_pa_tx.frame_interval = 20; //12.5ms */
    u_pa_tx.frame_interval = 32; //20ms
    u_pa_tx.phy_mode = 0;
    u_pa_tx.retry_num = 8;
    u_pa_tx.flow_mode = 0;
    /* u_pa_tx.retry_interval = 1250; */
    u_pa_tx.user_cb.event_cb = vble_padv_radio_tx_callback;

    log_info("\nrx_parm:\n");
    log_info("u_pa_rx.adv_name %s\n", &(u_pa_rx.adv_name));
    log_info("u_pa_rx.frame_len %d\n", u_pa_rx.frame_len);
    log_info("u_pa_rx.frame_interval %d\n", u_pa_rx.frame_interval);
    log_info("u_pa_rx.phy_mode %d\n", u_pa_rx.phy_mode);
    log_info("u_pa_rx.retry_num %d\n", u_pa_rx.retry_num);
    log_info("u_pa_rx.flow_mode %d\n", u_pa_rx.flow_mode);

    log_info("\n");
    log_info("\ntx_parm:\n");
    log_info("u_pa_tx.adv_name %s\n", &(u_pa_tx.adv_name));
    log_info("u_pa_tx.frame_len %d\n", u_pa_tx.frame_len);
    log_info("u_pa_tx.frame_interval %d\n", u_pa_tx.frame_interval);
    log_info("u_pa_tx.phy_mode %d\n", u_pa_tx.phy_mode);
    log_info("u_pa_tx.retry_num %d\n", u_pa_tx.retry_num);
    log_info("u_pa_tx.flow_mode %d\n", u_pa_tx.flow_mode);
}


