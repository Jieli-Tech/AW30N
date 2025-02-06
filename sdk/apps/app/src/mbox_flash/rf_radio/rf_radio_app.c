#include "rf_radio_app.h"
#include "trans_packet.h"
#include "key.h"
#include "msg.h"
#include "audio.h"
#include "power_api.h"
#include "rf_pa_port.h"
#include "hot_msg.h"
#include "tick_timer_driver.h"
#include "audio_adc_api.h"
#include "audio_dac_api.h"
#include "audio_dac_fade.h"
#include "audio_dac.h"
#include "test_app_config.h"
#include "decoder_api.h"

#include "rf_radio_app.h"
#include "connect_radio.h"
#include "padv_radio.h"


#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[rf_radio]"
#include "log.h"

#if (RF_RADIO_EN & BLE_EN & TESTE_BLE_EN)

#define RADIO_DEFAULT_ROLE_MASTER   0

u8 g_rf_radio_occur;
u32 g_rf_radio_softoff_jif;
u32 g_rf_radio_standby_jif_cnt;



void rra_timeout_reset(void)
{
    app_softoff_time_reset(g_rf_radio_softoff_jif);
    g_rf_radio_standby_jif_cnt = maskrom_get_jiffies() + 10;
}

static u16 rf_radio_key_filter(u8 key_status, u8 key_num, u8 key_type)
{
    u16 msg = rf_radio_key_msg_filter(key_status, key_num, key_type);
    if (msg != NO_MSG) {
        g_rf_radio_occur = 1;
        rra_timeout_reset();
    }
    return msg;

}

static void rf_radio_app_init(void)
{
    sysmem_write_api(SYSMEM_INDEX_SYSMODE, &work_mode, sizeof(work_mode));
    audio_rf_clr_buf();
    dac_sr_api(SR_DEFAULT);
    decoder_init();
    key_table_sel(rf_radio_key_filter);

}

static void rf_radio_app_uninit(void)
{
    audio_init();
    dac_init_api(SR_DEFAULT);
    key_table_sel(NULL);

}

/*----------------------------------------------------------------------------*/
/**@brief   对讲机应用主函数
   @param
   @return
   @author
   @note    主函数根据选择不同方案运行不同的子函数；

*/
/*----------------------------------------------------------------------------*/
void rf_radio_app(void)
{
    rf_radio_app_init();

#if PADVB_WL_MODE
    /* 广播对讲机 */
    rf_radio_padv_app();
#else
    /* 带连接对讲机 */
    rf_radio_connect_app();
#endif

    rf_radio_app_uninit();

}

#endif
