#include "encoder_mge.h"
#include "audio_adc_api.h"
#include "cpu.h"
#include "config.h"
#include "typedef.h"
#include "hwi.h"
/* #include "dev_manage.h" */
/* #include "fs_io.h" */
#include "vfs.h"
#include "circular_buf.h"
#include "a_encoder.h"
#include "mp3_encoder.h"
#include "clock.h"


#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[normal]"
#include "log.h"


cbuffer_t cbuf_adc AT(.rec_data);
static u8 adc_buff[7680] AT(.rec_data);//norfs擦写时间最长不超过80ms,最小缓存不小于sr*2*80ms

/* #if (0 == FPGA) */
#ifndef FPGA
sound_out_obj rec_sound;

#define START_ADC_RUN  rec_sound.enable |= (B_DEC_RUN_EN | B_REC_RUN)
#define STOP_ADC_RUN  rec_sound.enable &= ~(B_DEC_RUN_EN | B_REC_RUN)

_WEAK_
void rec_cbuf_init(void *cbuf_t)
{
    cbuf_init(cbuf_t, &adc_buff[0], sizeof(adc_buff));
}

void rec_phy_init(void)
{
    memset(&rec_sound, 0, sizeof(rec_sound));
    rec_cbuf_init(&cbuf_adc);
    /* cbuf_init(&cbuf_adc, &adc_buff[0], sizeof(adc_buff)); */
    rec_sound.p_obuf = &cbuf_adc;
    regist_audio_adc_channel(&rec_sound, (void *) kick_encode_api); //注册到DAC;
}
void rec_phy_suspend(void)
{
    unregist_audio_adc_channel(&rec_sound);
}

AT(.audio_a.text.cache.L2)
void kick_encode_api(void *obj)
{
    kick_encode_isr();
}
#endif



enc_obj *enc_hdl;

void start_encode(void)
{
    audio_adc_enable();
    mdelay(10);
    START_ADC_RUN;
}
void stop_encode_phy(ENC_STOP_WAIT wait, u8 has_file_write)
{
    enc_obj *obj = enc_hdl;
    u32 err;
    audio_adc_disable();
    STOP_ADC_RUN;
    log_info("stop encode\n");
    if (NULL == enc_hdl) {
        rec_phy_suspend();
        return;
    }
    obj->enable |= B_ENC_STOP;

    if (ENC_NEED_WAIT == wait) {
        log_info("stop encode A\n");
        u32 i = 0x10000;
        while (0 != cbuf_get_data_size(obj->p_ibuf) && (0 != i)) {
            if (obj->enable & B_ENC_FULL) {
                break;
            }
            kick_encode_isr();
            delay(100);
            i--;
        }
        if (has_file_write) {
            log_info("stop encode C\n");
            u32 i = 0x10000;
            while (0 != cbuf_get_data_size(obj->p_obuf) && (0 != i)) {
                if (obj->enable & B_ENC_FULL) {
                    break;
                }
                kick_wfile_isr();
                delay(100);
                i--;
            }
        }
    }

    log_info("stop encode D\n");
    obj->enable &= ~B_ENC_ENABLE;
    HWI_Uninstall(IRQ_SOFT1_IDX);
    if (has_file_write) {
        HWI_Uninstall(IRQ_SOFT2_IDX);
    }
    rec_phy_suspend();
    enc_hdl = 0;
}
void stop_encode(void *pfile, u32 dlen)
{
    stop_encode_phy(ENC_NEED_WAIT, 1);
    u32 flen = dlen;
    vfs_ioctl(pfile, FS_IOCTL_FILE_SYNC, (int)&flen);
}

/* static u32 recfil_index; */
void encoder_io(u32(*fun)(void *, void *, void *), void *pfile)
{
    s32 err;
    rec_phy_init();

    enc_hdl = (void *)fun(pfile, enc_input, enc_output);
    if (0 != enc_hdl) {
        enc_phy_init();
        enc_hdl->enable = B_ENC_ENABLE;
        start_encode();//adc_enable();
        log_info("encode succ: \n");
    } else {
        log_info("encode fail \n");
    }
    //while(1)clear_wdt();
}

#if 0
/*----------------------------------------------------------------------------*/
/**@brief   encode写中断hook函数
   @param   *hdl  : 录音设备句柄
   @return
   @author  chenzhuohao
   @note    若设备写入速度较慢导致看门狗复位，可通过该函数在写入前喂狗
   			注意：写入前喂狗可能使系统无法回到主循环,导致无法响应其他事件！
**/
/*----------------------------------------------------------------------------*/
void wfil_soft2_isr_hook(enc_obj *hdl)
{

}
#endif
