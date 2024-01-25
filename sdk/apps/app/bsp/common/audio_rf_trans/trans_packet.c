#pragma bss_seg(".rf_packet.data.bss")
#pragma data_seg(".rf_packet.data")
#pragma const_seg(".rf_packet.text.const")
#pragma code_seg(".rf_packet.text")
#pragma str_literal_override(".rf_packet.text.const")

#include "trans_packet.h"
#include "crc16.h"
#include "errno-base.h"
#include "audio2rf_send.h"
#include "rf_send_queue.h"

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[au2rf_pack]"
#include "log.h"

audio2rf_send_mge_ops *g_send_ops AT(.ar_trans_data);
extern volatile u32 rf_send_cnt;
static u8 g_pack_buf[SENDER_BUF_SIZE] AT(.ar_trans_data);

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
    g_send_ops = ops;
    local_irq_enable();
}

/*----------------------------------------------------------------------------*/
/**@brief   音频传输封包发送接口
   @param   type    :发送的数据类型
            data    :发送的数据buff
            data_len:发送的数据长度
            packet_buf:封包缓存buff
   @return  封包后的包长度
   @author
   @note    发送前会查询发送状态和可发送长度，条件未满足退出发送
*/
/*----------------------------------------------------------------------------*/
u16 ar_trans_pack(RADIO_PACKET_TYPE type, u8 *data, u16 data_len, u8 *packet_buf)
{
    u16 crc16_tmp;
    u16 slen = 0;

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

    memcpy(&packet_buf[0], &packet, sizeof(packet));
    slen += sizeof(packet);
    if (NULL != data) {
        memcpy(&packet_buf[sizeof(packet)], data, data_len);
        slen += data_len;
    }
    /* log_info("S cnt:%d type:%d len:%d crc:0x%x", packet.packet_index, packet.type, sizeof(RF_RADIO_PACKET) + packet.data_length, crc16_tmp); */
    return slen;
}

u32 audio2rf_send_packet(RADIO_PACKET_TYPE type, u8 *data, u16 data_len)
{
    u32 res = 0;
    /* bool is_in_isr = cpu_in_irq() */
    u8 *packet_data = &g_pack_buf[0];

    local_irq_disable();
    u16 packet_len = ar_trans_pack(type, data, data_len, packet_data);
    //packet_data & packet_len
    res = rf_send_push2queue(packet_data, packet_len);
    local_irq_enable();
    return res;
}
