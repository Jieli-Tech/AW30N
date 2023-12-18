#pragma bss_seg(".rf_packet.data.bss")
#pragma data_seg(".rf_packet.data")
#pragma const_seg(".rf_packet.text.const")
#pragma code_seg(".rf_packet.text")
#pragma str_literal_override(".rf_packet.text.const")

#include "trans_packet.h"
#include "crc16.h"
#include "errno-base.h"

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[au2rf_pack]"
#include "log.h"

static audio2rf_send_mge_ops *p_send_ops AT(.ar_trans_data);
extern volatile u32 rf_send_cnt;

#define HEAD_DATA_SEND_TOGETHER     0
#if HEAD_DATA_SEND_TOGETHER
static u8 tmp_send_buf[SENDER_BUF_SIZE] AT(.ar_trans_data);
#endif

/*----------------------------------------------------------------------------*/
/**@brief   音频无线传输接口注册
   @param   ops:发送相关接口注册,结构体原型为audio2rf_send_mge_ops
   @return  无
   @author
   @note    注册接口涉及发送、状态获取、可发送长度获取
*/
/*----------------------------------------------------------------------------*/
void audio2rf_send_register(void *ops)
{
    local_irq_disable();
    p_send_ops = ops;
    local_irq_enable();
}

/*----------------------------------------------------------------------------*/
/**@brief   音频传输封包发送接口
   @param   type    :发送的数据类型
            data    :发送的数据buff
            data_len:发送的数据长度
   @return  0:成功   非0:失败，错误值请查看errno-base.h
   @author
   @note    发送前会查询发送状态和可发送长度，条件未满足退出发送
*/
/*----------------------------------------------------------------------------*/
u32 ar_trans_pack(RADIO_PACKET_TYPE type, u8 *data, u16 data_len)
{
    if (NULL == p_send_ops) {
        return E_AU2RF_OBJ_NULL;
    }

    u32 rf_status = p_send_ops->check_status();
    if (rf_status == 0) {
        /* rf无法正常通信 */
        return E_AU2RF_RF_OFFLINE;
    }

    u32 can_send_len = p_send_ops->get_valid_len();
    if (can_send_len < (sizeof(RF_RADIO_PACKET) + data_len)) {
        /* 可发送长度不足头+数据,退出本次发送 */
        /* putchar('@'); */
        log_info("can_send_len:%d  %d\n", can_send_len, (sizeof(RF_RADIO_PACKET) + data_len));
        return E_AU2RF_RF_BUSY;
    }

    u16 crc16_tmp;
    u32 err = 0;

    RF_RADIO_PACKET packet = {0};
    packet.header      = PACKET_HEAD;
    packet.type        = type;
    packet.data_length = data_len;
    packet.packet_index = rf_send_cnt++;
    crc16_tmp = CRC16_with_initval(&(packet.type), RADIO_PACKET_CRC_LEN, 0);
    if (NULL != data) {
        crc16_tmp = CRC16_with_initval(data, data_len, crc16_tmp);
    }
    packet.crc8_l = crc16_tmp & 0xff;

#if HEAD_DATA_SEND_TOGETHER
    memcpy(&tmp_send_buf[0], &packet, sizeof(packet));
    if (NULL != data) {
        memcpy(&tmp_send_buf[sizeof(packet)], data, data_len);
    }
    err = p_send_ops->send((u8 *)&tmp_send_buf[0], sizeof(packet) + data_len);
    /* err = p_send_ops->send((u8 *)&err, 1); */
    if (err) {
        return E_AU2RF_SNED_DATA_ERR;
    } else {
        return 0;
    }
#else

    local_irq_disable();
    /* 数据头压入发送缓存 */
    err = p_send_ops->send((u8 *)&packet, sizeof(RF_RADIO_PACKET));
    if (err) {
        /* log_info("001 err:0x%x\n",err); */
        local_irq_enable();
        return E_AU2RF_SNED_HEAD_ERR;
    }
    /* 数据压入发送缓存 */
    if (NULL != data) {
        err = p_send_ops->send((u8 *)data, data_len);
        if (err) {
            /* log_info("002 err:0x%x\n",err); */
            local_irq_enable();
            return E_AU2RF_SNED_DATA_ERR;
        }
    }
    local_irq_enable();

#endif
    /* log_info("S cnt:%d type:%d len:%d crc:0x%x", packet.packet_index, packet.type, sizeof(RF_RADIO_PACKET) + packet.data_length, crc16_tmp); */
    return 0;
}

u32 audio2rf_send_packet(RADIO_PACKET_TYPE type, u8 *data, u16 data_len)
{
    return ar_trans_pack(type, data, data_len);
}
