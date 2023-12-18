#ifndef __LIB_MP3_ST_H__
#define __LIB_MP3_ST_H__

#define MP3_ST_DBUF_SIZE        (0x3c40)
#define MP3_ST_TRACK            (0) // B_STEREO
#define MP3_ST_OUTPUT_MAX_SIZE  (32 * 2 * 1)//32 * 2byte * 1ch

#define MP3_ST_INPUT            mp_input
#define MP3_ST_FORMAT_CHECK_ENTER()
#define MP3_ST_FORMAT_CHECK_EXIT()

#define MP3_ST_DEC_CONFING() \
    do { \
        parm_nchv.ch_value = FAST_LR_OUT; \
        ops->dec_confing(MP3_ST_CAL_BUF, CMD_SET_DECODE_CH, &parm_nchv);/*配置解码输出通道*/ \
    }while(0)

#endif
