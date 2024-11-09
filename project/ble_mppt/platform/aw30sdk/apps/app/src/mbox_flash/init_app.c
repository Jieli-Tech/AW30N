
#pragma bss_seg(".init_app.data.bss")
#pragma data_seg(".init_app.data")
#pragma const_seg(".init_app.text.const")
#pragma code_seg(".init_app.text")
#pragma str_literal_override(".init_app.text.const")

#include "init.h"
#include "device.h"
#include "vfs.h"
#include "msg.h"
#include "clock.h"
#include "key.h"
#include "ui_api.h"
#if TCFG_CHARGE_ENABLE
#include "charge.h"
#endif
#include "audio.h"
#include "audio_dac_api.h"
#include "adc_api.h"
#include "pa_mute.h"
#include "src_api.h"
#include "app_config.h"
#include "flash_init.h"
#include "sine_play.h"
#include "app_power_mg.h"
#include "gpio.h"
#include "power_interface.h"
#include "power_api.h"
#include "audio_adc.h"
#if HAS_NORFS_EN
#include "nor_fs.h"
#endif

#if defined(AUDIO_HW_EQ_EN) && (AUDIO_HW_EQ_EN)
#include "effects_adj.h"
#endif

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[init]"
#include "log.h"

void app_system_init(void)
{
    //adc_init();

}

