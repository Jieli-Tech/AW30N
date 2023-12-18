#pragma bss_seg(".ar_mge.data.bss")
#pragma data_seg(".ar_mge.data")
#pragma const_seg(".ar_mge.text.const")
#pragma code_seg(".ar_mge.text")
#pragma str_literal_override(".ar_mge.text.const")

#include "audio_rf_mge.h"
#include "cpu.h"
#include "config.h"
#include "typedef.h"
#include "vfs.h"
#include "msg.h"
#include "crc16.h"
#include "circular_buf.h"

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[rf_basic]"
#include "log.h"

volatile u8 rf_radio_status AT(.ar_trans_data);

void set_radio_status(RADIO_STATUS status)
{
    log_info("\n\n\tsta:%d->%d\n", rf_radio_status, status);
    local_irq_disable();
    rf_radio_status = status;
    local_irq_enable();
}

/*----------------------------------------------------------------------------*/
/**@brief   音频传输模块变量初始化
   @param
   @return
   @author
   @note    音频传输模块部分变量已overlay复用，使用前需要清空内存
*/
/*----------------------------------------------------------------------------*/
extern const u32 ar_trans_data_start[];
extern const u32 ar_trans_data_end[];
void audio_rf_clr_buf(void)
{
    u32 buf_len = (u32)&ar_trans_data_end[0] - (u32)&ar_trans_data_start[0];
    memset((void *)&ar_trans_data_start[0], 0, buf_len);
    /* log_info("start:0x%x end:0x%x len:0x%x\n", \ */
    /*          (u32)&ar_trans_data_start[0], \ */
    /*          (u32)&ar_trans_data_end[0], \ */
    /*          buf_len); */
}
