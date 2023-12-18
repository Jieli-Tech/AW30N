#ifndef __AUADC_2_USBMIC_H__
#define __AUADC_2_USBMIC_H__
#include "typedef.h"
#include "sound_mge.h"

sound_out_obj *auadc_mic_open(u32 sr, u32 frame_len, u32 ch, void **ppsrc, void **pkick);
void auadc_mic_close(void **ppsrc);
#endif
