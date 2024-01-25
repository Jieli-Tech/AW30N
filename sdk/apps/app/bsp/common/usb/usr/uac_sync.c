
#pragma bss_seg(".usb_audio_if.data.bss")
#pragma data_seg(".usb_audio_if.data")
#pragma const_seg(".usb_audio_if.text.const")
#pragma code_seg(".usb_audio_if.text")
#pragma str_literal_override(".usb_audio_if.text.const")
#include "cpu.h"
#include "config.h"
#include "typedef.h"
#include "usb/device/uac_audio.h"
#include "usb/usr/usb_audio_interface.h"
#include "usb/usr/usb_mic_interface.h"
#include "usb/usr/uac_sync.h"
#include "iir.h"

#define LOG_TAG_CONST       USB
#define LOG_TAG             "[SYNC]"
#include "log.h"
#include "uart.h"



void uac_sync_init(uac_sync *sync, s32 sr)
{
    if (NULL == sync) {
        return;
    }
    memset(sync, 0, sizeof(uac_sync));
    /* sync->sr_curr = 0; */
    /* sync->pe_cnt  = 0; */
    /* sync->pe5_cnt  = 0; */
    sync->pe5_step  = 20;
    sync->pe5_dec  = 10;
    sync->pe_change_cnt = 0;
    /* sync->pe_inc_data = 0; */
    /* sync->pe_sub_data = 0; */
    sync->last_pe = 0xff;
    /* sync->last_sr = 0; */
    sync->x_step = sr * 5 / 10000;

    initLowPassFilter(&sync->f_percent, 85);

}
static u32 uac_sr_milli(u32 sr)
{
#if 0
    u32 tmp;

    if (sr > sr_curr) {
        tmp = sr - sr_curr;
    } else {
        tmp = sr_curr - sr ;
    }
    u32 milli = tmp * 1000 / sr;
    return milli;
#else
    return 0;
#endif

}



void uac_1s_sync(void)
{
    return;
#if 0
    u32 sr_1s = uac_speaker_stream_1s();
    sr_curr = sr_1s;
    if (0 == sr_1s) {
        return;
    }
    if (0 == uac_is_need_sync()) {
        return;
    }
    sound_in_obj *p_src_si = usb_src_obj->p_si;
    SRC_STUCT_API *p_ops =  p_src_si->ops;
    if (0 != sync->last_sr) {
        u32 milli = uac_sr_milli(sync->last_sr);
        if (milli < 4) {
            return;
        }
    }
    log_info(" 1S SR %d\n", sr_1s);
    /*uac_sync_parm =  p_ops->config(p_src_si->p_dbuf, SRC_CMD_INSR_SET, (void *)sr_1s); */
    sync->last_sr = sr_1s;
#endif
}

static void uac_inc_sync_pe_reset(uac_sync *sync)
{
    sync->pe5_step += 40;
    sync->pe5_cnt = sync->pe5_step;
    /* sync->pe_inc_data = 0; */
    /* sync->pe_sub_data = 0; */
    /* sync->pe_cnt = 0; */
    sync->last_pe = 0xff;
}

void uac_inc_sync_one(EFFECT_OBJ *e_obj, u32 percent, uac_sync *sync)
{
    /* if (0 == uac_is_need_sync()) { */
    /* return; */
    /* } */
    sound_in_obj *p_src_si = e_obj->p_si;
    SRC_STUCT_API *p_ops =  p_src_si->ops;

    char c = 0;
    char d = 0;
    s32 step = 0;

    percent = applyLowPassFilter(&sync->f_percent, percent);
#if 1
    bool flag_out_range = 1;
    if (percent > 58) {
        if (0 == sync->pe5_cnt) {
            if (0 != sync->pe_cnt) {
                step = ((s16)sync->pe_inc_data * 8) / sync->pe_cnt;
            }
            if (0 == sync->pe_sub_data) {
                if (0 == step) {
                    step = 1;
                } else {
                    step = step * sync->x_step;
                }

            }
            uac_inc_sync_pe_reset(sync);
            c = 100 + step;
        }
        /* sync->sync_cnt = 0; */
    } else if (percent < 42) {
        if (0 == sync->pe5_cnt) {
            if (0 != sync->pe_cnt) {
                step = (((s16)sync->pe_sub_data * 8) / sync->pe_cnt);

            }
            if (0 == sync->pe_inc_data) {
                if (0 == step) {
                    step = -1;
                } else {
                    step = 0 - (step * sync->x_step);
                }

            }
            uac_inc_sync_pe_reset(sync);
            c = 100 + step;
        }
    } else {
        flag_out_range = 0;
        sync->pe5_step = 0;
    }
    if ((sync->last_pe < 100) && (percent == sync->last_pe)) {
        if (2 < sync->pe5_dec) {
            sync->pe5_dec -= 2;
        }
    } else {
        sync->pe5_dec = 10;
    }
    if (0 != sync->pe5_cnt) {
        u32 tmp = (sync->pe5_cnt < sync->pe5_dec ? sync->pe5_cnt : sync->pe5_dec);
        sync->pe5_cnt -= tmp;
    }
#endif


#if 1
    if (sync->last_pe <= 100) {
        if (percent < sync->last_pe) {	//降
            if (0 != sync->pe_inc_data) {
                sync->pe_inc_data = 0;
                sync->pe_cnt = sync->pe_change_cnt;
                sync->pe_change_cnt = 0;
            }
            sync->pe_sub_data += (sync->last_pe - percent);
        } else if (percent > sync->last_pe) {	//升

            if (0 != sync->pe_sub_data) {
                /* sync->pe_inc_data = 0; */
                sync->pe_sub_data = 0;
                sync->pe_cnt = sync->pe_change_cnt;
                sync->pe_change_cnt = 0;
            }
            sync->pe_inc_data += (percent - sync->last_pe);
        }
    }
    sync->pe_cnt++;
    sync->pe_change_cnt++;

    if ((sync->last_pe <= 100) && (0 == flag_out_range)) {

        if (sync->pe_sub_data > 1) {
            step = (sync->pe_sub_data * 8) / sync->pe_cnt;
            if (step > 0) {
                step = (step * sync->x_step);
            } else {
                step =  1;
            }
            step =  0 - step;
            d = 100 + step;
            if (percent > 10) {
                sync->pe_sub_data = 0;
                sync->pe_cnt = 0;
            }
        } else if (sync->pe_inc_data > 1) {
            step = (sync->pe_inc_data * 8) / sync->pe_cnt;
            if (step > 0) {
                step = (step * sync->x_step);
            } else {
                /* step =  sync->x_step / 2; */
                step =  1;
            }
            d = 100 + step;
            if (percent < 90) {
                sync->pe_inc_data = 0;
                sync->pe_cnt = 0;
            }
        }

    }
#endif


    if (0 != step) {
        sync->uac_sync_parm = p_ops->config(
                                  p_src_si->p_dbuf,
                                  SRC_CMD_INSR_INC_SET,
                                  (void *)step
                              );
    }
    if (c == 0) {
        c = 100;
    }
    /* log_char(c); */
    if (d == 0) {
        d = 100;
    }
    /* log_char(d); */

    sync->last_pe = percent;
}


extern uac_sync uac_spk_sync;
extern uac_sync uac_mic_sync;
void uac_inc_sync(void)
{

    EFFECT_OBJ *e_obj = NULL;
    u32 percent;
    /* log_char('a'); */
#if ( TCFG_PC_ENABLE && (USB_DEVICE_CLASS_CONFIG & MIC_CLASS))
#if TCFG_MIC_SRC_ENABLE
    e_obj = uac_mic_percent(&percent);
    if (NULL != e_obj) {
        /* log_char('b'); */
        uac_inc_sync_one(e_obj, percent, &uac_mic_sync);
    }
    /* log_char('c'); */
#endif
#endif

#if TCFG_SPK_SRC_ENABLE
    e_obj = uac_spk_percent(&percent);
    if (NULL != e_obj) {
        /* log_char('d'); */
        uac_inc_sync_one(e_obj, percent, &uac_spk_sync);
    }
    /* log_char('e'); */
#endif
}

