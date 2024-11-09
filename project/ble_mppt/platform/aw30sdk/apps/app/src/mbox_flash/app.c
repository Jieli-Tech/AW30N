#pragma bss_seg(".app.data.bss")
#pragma data_seg(".app.data")
#pragma const_seg(".app.text.const")
#pragma code_seg(".app.text")
#pragma str_literal_override(".app.text.const")

#include "app_modules.h"
#include "app_config.h"
#include "config.h"
#include "common.h"
#include "app.h"
#include "msg.h"
#include "sys_memory.h"
#include "vm_sfc.h"

#include "music_play.h"
#include "usb_slave_mode.h"
#include "linein_mode.h"
#include "record_mode.h"
#include "simple_decode.h"
#include "midi_dec_mode.h"
#include "midi_keyboard_mode.h"
#include "loudspk_mode.h"
#include "rtc_mode.h"
#if RF_RADIO_EN
#include "rf_radio_app.h"
#endif
#include "rc_app.h"
#include "softoff_mode.h"

#include "jiffies.h"
#include "ui_api.h"
#include "key.h"
#include "audio_dac_api.h"
#include "usb_audio_interface.h"
#include "audio.h"
#include "audio_adc.h"
#include "device.h"
#include "otg.h"
#if TCFG_CHARGE_ENABLE
#include "charge.h"
#endif
#include "pa_mute.h"
#include "device_app.h"
#include "adc_api.h"
#include "usb/host/usb_host.h"
#include "usb/device/usb_stack.h"
#include "usb/otg.h"
#include "wdt.h"
#include "bsp_loop.h"
#include "app_power_mg.h"
#include "sine_play.h"
#include "init_app.h"
#if defined (BLE_EN) && (BLE_EN)
#include "bt_ble.h"
#endif
#if SYS_TIMER_EN
#include "sys_timer.h"
#endif

#define LOG_TAG_CONST       APP
#define LOG_TAG             "[mbox_app]"
#include "log.h"

//u8 work_mode;
//AT(.tick_timer.text.cache.L2)
//void tick_timer_ram_loop(void)
//{
//
//}
//
//extern u8 tick_cnt;
//void app_timer_loop(void)
//{
//    tick_cnt++;
//}
//
//u32 get_up_suc_flag();
//void app_main(void)
//{
//    log_info("BLE MPPT App Start\n");
//    if (get_up_suc_flag()) {
//        log_info("----device update end----\n");
//        wdt_close();
//        while (1);
//    }
//
//    vm_isr_response_index_register(IRQ_TICKTMR_IDX);//vm擦写flash时响应tick_timer_ram_loop()函数
//    delay_10ms(50);//等待系统稳定
//    pa_mute(0);
//
//    sysmem_read_api(SYSMEM_INDEX_SYSMODE, &work_mode, sizeof(work_mode));
//    while (1) {
//        wdt_clear();
//        clear_all_message();
//        os_time_dly(1);
//    }
//}
//
//void app_custom(void)
//{
//    log_info("%s() %d\n", __func__, __LINE__);
//    app_system_init();
//    app_main();
//}

//void app(void)
//{
//    app_custom();
//
//    log_error("No app running!!!\n");
//    while (1) {
//        wdt_clear();
//    }
//}
