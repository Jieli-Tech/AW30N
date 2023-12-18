#pragma bss_seg(".ar_unpack.data.bss")
#pragma data_seg(".ar_unpack.data")
#pragma const_seg(".ar_unpack.text.const")
#pragma code_seg(".ar_unpack.text")
#pragma str_literal_override(".ar_unpack.text.const")

#include "trans_unpacket.h"
#include "typedef.h"
#include "crc16.h"
#include "errno-base.h"

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[au2rf_upack]"
#include "log.h"

/*----------------------------------------------------------------------------*/
/**@brief   音频传输接收拆包
   @param   ops     :状态机管理句柄
            buff    :接收数据buff
            len     :接收数据长度
            pcnt    :有效数据偏移指针
   @return  true:找到有效数据   false:未找到有效数据
   @author
   @note    状态机解析封包的数据头，并分发给下一级
*/
/*----------------------------------------------------------------------------*/
bool ar_trans_unpack(rev_fsm_mge *ops, u8 *buff, u16 len, u32 *pcnt)
{
    u32 stream_index = *pcnt;
    while (stream_index < len) {
        switch (ops->status) {
        case REV_HEADER_0x55:
            /* log_info("REV_HEADER_0x55:0x%x\n", buff[stream_index]); */
            if (0x55 == buff[stream_index]) {
                ops->status = REV_HEADER_0xaa;
                ops->got_length = 0;
                ops->length = 0;
                ops->crc_bk = 0;
                ops->crc = 0;
            }
            stream_index++;
            /* 未收得到数据头，等待数据头中 */
            break;
        case REV_HEADER_0xaa:
            /* log_info("REV_HEADER_0xaa:0x%x\n", buff[stream_index]); */
            if (0xaa == buff[stream_index]) {
                ops->status = REV_CRC;
            } else if (0x55 == buff[stream_index]) {
                break;
            } else {
                ops->status = REV_HEADER_0x55;
            }
            stream_index++;
            break;
        case REV_CRC:
            /* log_info("REV_CRC:0x%x\n", buff[stream_index]); */
            ops->crc = buff[stream_index];
            ops->status = REV_TYPE;
            stream_index++;
            break;
        case REV_TYPE:
            /* log_info("REV_TYPE:0x%x\n", buff[stream_index]); */
            ops->type = buff[stream_index];
            ops->status = REV_LEN_L;
            ops->crc_bk = CRC16_with_initval(&buff[stream_index], 1, 0);
            stream_index++;
            break;
        case REV_LEN_L:
        case REV_LEN_H: {
            /* log_info("REV_LEN_L:0x%x\n", buff[stream_index]); */
            u8 len_num = ops->status - REV_LEN_L;
            u8 *plen = (u8 *)&ops->length;
            plen[len_num] = buff[stream_index];
            ops->status += 1;
            ops->crc_bk = CRC16_with_initval(&buff[stream_index], 1, ops->crc_bk);
            stream_index++;
            /* if ((REV_LEN_H + 1) ==  ops->status) { */
            /*     log_debug("recv data len:%d\n", ops->length); */
            /* } */
        }
        break;
        case REV_PINDEX_0:
        case REV_PINDEX_1:
        case REV_PINDEX_2:
        case REV_PINDEX_3: {
            u8 pindex_num = ops->status - REV_PINDEX_0;
            /* log_info("REV_PINDEX_%d:0x%x\n", pindex_num, buff[stream_index]); */
            u8 *pindex = (u8 *)&ops->packet_index;
            pindex[pindex_num] = buff[stream_index];
            ops->status++;
            ops->crc_bk = CRC16_with_initval(&buff[stream_index], 1, ops->crc_bk);
            /* if ((REV_PINDEX_3 + 1) == ops->status) { */
            /* log_debug("recv packet index:%d\n", ops->packet_index); */
            /* log_debug("crc16_bk:0x%x\n", ops->crc_bk); */
            /* } */
            if (ops->type == AUDIO2RF_STOP_PACKET) {
                ops->status = REV_HEADER_0x55;
                goto __recv_data_succ_exit;
            }
            stream_index++;
        }
        break;
        case REV_DATA: {
            /* 已收得到数据头，收取数据中 */
            u16 remain_len = ops->length - ops->got_length;//数据帧还没取的数据长度
            u16 res_len = len - stream_index;//本次发送还剩余的数据长度
            u16 rlen;
            if (0 == ops->got_length) {
                /* 第一次拿数 */
                rlen = (ops->length > res_len) ? res_len : ops->length;
            } else {
                /* 非第一次拿数 */
                rlen = (remain_len > res_len) ? res_len : remain_len;
            }
            ops->crc_bk = CRC16_with_initval(&buff[stream_index], rlen, ops->crc_bk);
            ops->got_length += rlen;

            if (ops->got_length >= ops->length) {
                /* log_debug("ops->crc_bk:0x%x 0x%x\n", ops->crc_bk, ops->crc); */
                if ((ops->crc_bk & 0xff) == ops->crc) {
                    /* log_debug("recv packet succ!\n"); */
                    ops->status = REV_HEADER_0x55;
                    goto __recv_data_succ_exit;
                } else {
                    /* log_debug("recv packet fail!\n"); */
                }
                ops->status = REV_HEADER_0x55;
            } else {

            }
            stream_index += ops->got_length;
        }
        break;
        }
    }
    /* log_debug("index:%d status:%d\n", stream_index, ops->status); */
    *pcnt = stream_index;
    return false;

__recv_data_succ_exit:
    *pcnt = stream_index;
    return true;
}
