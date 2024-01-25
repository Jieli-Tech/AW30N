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
static u32 ar_trans_unpack_fsm(rev_fsm_mge *ops, u8 *buff, u16 len, u32 *pcnt)
{
    u32 res = TUR_HEAD_RECVING;
    u32 stream_index = 0;
    while (stream_index < len) {
        switch (ops->status) {
        case REV_HEADER_0x55:
            /* log_info("REV_HEADER_0x55:0x%x\n", buff[stream_index]); */
            if (0x55 == buff[stream_index]) {
                /* memset((u8 *)ops, 0, sizeof(rev_fsm_mge)); */
                ops->type = 0;
                ops->packet_index = 0;
                ops->got_length = 0;
                ops->length = 0;
                ops->crc_bk = 0;
                ops->crc = 0;
                ops->status = REV_HEADER_0xaa;
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
            /*     log_debug("ridx:%d t:%d s:%d l:%d crc:0x%x 0x%x\n", ops->packet_index, ops->type, ops->status, ops->length, ops->crc_bk, ops->crc); */
            /*     log_debug("b:0x%x %d %d", buff, len, stream_index); */
            /* } */
            if ((ops->length == 0) && ((ops->crc_bk & 0xff) == ops->crc)) {
                ops->status = REV_HEADER_0x55;
                stream_index++;
                res = TUR_DATA_CORRECT;
                goto __recv_data_exit;
            }
            stream_index++;
        }
        break;
        case REV_DATA: {
            /* 已收得到数据头，收取数据中 */
            u16 remain_len = ops->length - ops->got_length;//数据帧还没取的数据长度
            u16 res_len = len - stream_index;//本次发送还剩余的数据长度
            /* log_info("REV_DATA:%d %d %d-%d\n", ops->length, ops->got_length, remain_len, res_len); */
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
            stream_index += rlen;

            if (ops->got_length >= ops->length) {
                ops->status = REV_HEADER_0x55;
                /* log_debug("ops->crc_bk:0x%x 0x%x\n", ops->crc_bk, ops->crc); */
                if ((ops->crc_bk & 0xff) == ops->crc) {
                    /* log_debug("recv packet succ!\n"); */
                    res = TUR_DATA_CORRECT;
                    goto __recv_data_exit;
                } else {
                    /* log_debug("recv packet fail!\n"); */
                    res = TUR_DATA_WRONG;
                    goto __recv_data_exit;
                }
            } else {
                res = TUR_DATA_RECVING;
            }
        }
        break;
        }
    }

__recv_data_exit:
    /* log_debug("index:%d status:%d\n", stream_index, ops->status); */
    *pcnt = stream_index;
    return res;
}

bool ar_trans_unpack(rev_fsm_mge *ops, void *input, u16 inlen,  u16 *offset)
{
    data_cache_mge *cache = &ops->cache;

    u32 len, rev_status, strm_offset;
    bool res = false;

    rev_status = ar_trans_unpack_fsm(ops, input, inlen, &strm_offset);
    *offset = strm_offset;

    if (ops->got_length > DATA_CACHE_BUF_SIZE) {
        /* log_error("cache_buf < data_length! %d %d\n", ops->length, DATA_CACHE_BUF_SIZE); */
        return false;
    }

    switch (rev_status) {
    case TUR_DATA_CORRECT:
        res = true;
    case TUR_DATA_RECVING:
        len = ops->got_length - cache->offset;
        memcpy(&cache->buf[cache->offset], &input[strm_offset - len], len);
        cache->offset += len;
        break;
    case TUR_DATA_WRONG:
        /* 数据出错，清空缓存 */
        memset((void *)cache, 0, sizeof(data_cache_mge));
    default:
        break;
    }

    return res;
}

/*----------------------------------------------------------------------------*/
/**@brief   应用接收数据分发处理
  @param   ops         :状态机管理句柄
buff        :本次接收数据buff
packet_len  :本次接收数据长度
@return
@author
@note
*/
/*----------------------------------------------------------------------------*/
#include "rf2audio_recv.h"
static void packet_2_app(rev_fsm_mge *ops, RADIO_PACKET_TYPE type, u8 *data, u16 len)
{
    switch (type) {
    case AUDIO2RF_DATA_PACKET:
        rf2audio_receiver_write_data(ops->dec_obj, data, len);
        break;
    default:
        packet_cmd_post(ops->cmd_pool, type, data, len);
        break;
    }
}

int unpack_data_deal(rev_fsm_mge *ops, u8 *buff, u16 packet_len)
{
    data_cache_mge *cache = &ops->cache;

    bool res;
    u8 unpack_cnt = 0;
    u16 used_len;

    while (packet_len) {
        res = ar_trans_unpack(ops, buff, packet_len, &used_len);
        buff += used_len;
        packet_len -= used_len;
        if (false == res) {
            continue;
        }

        u8 *pdata = &cache->buf[0];
        u16 data_len = ops->length;
        /* log_info("R cnt:%d type:%d len:%d crc:0x%x", ops->packet_index, ops->type, sizeof(RF_RADIO_PACKET) + data_len, ops->crc); */

        packet_2_app(ops, ops->type, pdata, data_len);
        memset((void *)cache, 0, sizeof(data_cache_mge));
        unpack_cnt++;
        if (unpack_cnt >= 5) {
            log_info("recv_data >= 5 packets!\n");
        }
    }

    return 0;
}

s32 packet_cmd_post(void *p_pool, RADIO_PACKET_TYPE cmd, u8 *data, u16 data_len)
{
    u8 need_len[1];
    need_len[0] = 1 + 1 + data_len;
    u32 ava_len = _cbuf_get_available_space((cbuffer_t *)p_pool);
    if (need_len[0] > ava_len) {
        return E_PACKET_FULL;
    }
    u32 res0 = cbuf_write(p_pool, &need_len[0], 1);
    u32 res1 = cbuf_write(p_pool, &cmd, 1);
    u32 res2  = cbuf_write(p_pool, data, data_len);
    if ((0 == res0) || (0 == res1) || (0 == res2)) {
        log_error("packet can't post cmd");
        //*post_event(); //通知应用层状态出错并处理
        return E_PACKET_WRITE;
    }
    return 0;
}


u32 packet_cmd_get(void *p_pool, u8 *packet, u8 packet_len)
{
    if (NULL == p_pool) {
        return 0;
    }
    if (NULL == packet) {
        return 0;
    }

    u32 clen = cbuf_read(p_pool, &packet[0], 1);
    if (0 == clen) {
        return 0;
    }
    u32 rlen = packet[0] < packet_len ? packet[0] : packet_len;
    u32 remain_len = packet[0] - rlen;

    rlen--;
    u32 len = cbuf_read(p_pool, &packet[1], rlen);
    /* log_info("%s() 1:%d 0:%d r:%d\n", __func__, __LINE__, packet[1], packet[0], remain_len); */
    if (len != rlen) {
        return 0;
    }
    if (0 != remain_len) {
        cbuf_read_updata(p_pool, remain_len);
    }
    return len + clen;
}

