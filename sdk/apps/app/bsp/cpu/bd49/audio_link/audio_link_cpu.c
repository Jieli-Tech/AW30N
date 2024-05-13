#include "app_modules.h"
#include "app_config.h"
#include "audio_link.h"
#include "audio_link_api.h"
#include "audio_dac_api.h"
#include "audio_adc_api.h"
#include "audio_adc.h"
#include "audio_dac.h"
#include "gpio.h"

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[normal]"
#include "log.h"


/* >>>>>>>>>>>>>>>>>>>>>>>tx */
u32 alink_tx_buf[ALINK_DMA_LEN / 4];

/* IO发送的数据写在BUF里 */
void alink_tx_cb(u8 ch, u8 *buf, u32 len)
{
    fill_dac_fill_phy(buf, len);
    uac_audio_dac_percent(ch);

}

ALINK_CH_CFG alnk_ch_cfg_tx = {
    .enable		= 1,
    .data_io	= ALINK_DATA_IO0,					//data IO配置
    .dir		= ALINK_DIR_TX, 				//通道传输数据方向: Tx, Rx
    .buf		= &alink_tx_buf[0],					//dma buf地址
    .isr_cb		= alink_tx_cb,
};


void alink_mode_init()
{
    log_info("alink_mode_init\n");
    audio_link_init_channel_api(&alnk_ch_cfg_tx);
    audio_link_en_api(ALINK_MODULE_OPEN);
}



void alink_mode_uninit()
{
    audio_link_uninit_channel_api(&alnk_ch_cfg_tx);
}



void set_audio_link_sr(u32 sr)
{
    u16 ret = audio_link_set_sr_api(sr);
    log_info("set_audio_link_sr sr %d\n", ret);
}

void alink_phy_vol(u16 dac_l, u16 dac_r)
{

}

/* <<<<<<<<<<<<<<<<<<<<<<<tx */


/* >>>>>>>>>>>>>>>>>>>>>>>rx */

u32 alink_rx_buf[ALINK_DMA_LEN / 4];
void alink_rx_cb(u8 ch, u8 *buf, u32 len)
{
    fill_audio_adc_fill_phy(buf, len);
    uac_audio_adc_percent(ch);
}

ALINK_CH_CFG alnk_ch_cfg_rx = {
    .enable		= 1,
    .data_io	= ALINK_DATA_IO1,					//data IO配置
    .dir		= ALINK_DIR_RX, 				//通道传输数据方向: Tx, Rx
    .buf		= &alink_rx_buf[0],					//dma buf地址
    .isr_cb		= alink_rx_cb,
};


void auadc_open_alink(void)
{
    audio_link_init_channel_api(&alnk_ch_cfg_rx);
    audio_link_en_api(ALINK_MODULE_OPEN);

}


u32 audio_link_phy_init(void *p_adc, u32 sr, u32 con, u32 throw)
{
    return 0;
}
/* <<<<<<<<<<<<<<<<<<<<<<<rx */
