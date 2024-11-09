#ifndef __AUDIO_APA_H__
#define __AUDIO_APA_H__

#include "typedef.h"

#define AUDAC_CHANNEL_TOTAL   3

//DSM 模式


#define APA_PLL320M          1
#define APA_PLL240M          2
#define APA_HSBCLK           3

#define APA_MODE_BTL         (0 << 11)
#define APA_MODE_PBTL        (1 << 11)

#define APA_MODE1_NOR         (0 << 9)
#define APA_MODE2_SNR         (1 << 9)
#define APA_MODE3_THD         (2 << 9)
#define APA_MODE_BITS         (2 << 9)

#define APA_MUTE             (1 << 8)

#define APA_PND              (1 << 7)
#define APA_CPND             (1 << 6)
#define APA_IE_EN            (1 << 5)
#define APA_PWM_EN           (1 << 4)
#define APA_DSM_EN           (1 << 3)







//apa 输出采样率
#define APA_SR48K    (0<<0)
#define APA_SR44K1   (1<<0)
#define APA_SR32K    (2<<0)
#define APA_SRBITS   (7<<0)
// #define DAC_SRBITS   APA_SRBITS

#define  CON0_SR_PARA  ((0x3ff <<22) | (0xff<<16))
#define  CON1_SR_PARA  ((0x1ff <<10) | (0x3ff<<0))


#define APA_CLK_DEFAULT      APA_PLL240M
#define APA_MODE_DEFAULT     APA_MODE2_SNR
#define DAC_DEFAULT          (APA_IE_EN| APA_MODE_DEFAULT)

// #define DAC_TRACK_NUMBER   1
#define APA_TRACK_NUMBER   1

#define audio_clk_init()  SFR(JL_CLOCK->CLK_CON2,  0,  2,  APA_CLK_DEFAULT)

// #define SR_DEFAULT  32000



typedef struct _APA_CTRL_HDL {
    void *buf0;
    void *buf1;
    u16 sp_len;
    u16 con0;
} APA_CTRL_HDL;

void apa_resource_init(const APA_CTRL_HDL *ops);

void apa_mode_init(void);
void apa_init(u32 sr, u32 delay_flag);
void apa_off_api(void);
void apa_sr_api(u32 sr);



void apa_clr_buf(void);
void apa_trim_api(void);
void apa_analog_open_api(u32 delay_flag);
void apa_phy_vol(u16 dac_l, u16 dac_r);

void apa_analog_open(void);
void apa_analog_close(void);
void apa_phy_init(u32 sr_sel);
void apa_phy_off(void);
u32 apa_sr_lookup(u32 sr);
u32 apa_sr_read(void);
u32 apa_sr_set(u32 sr);
void apa_hardware_mute(u8 mute);
void single_apa_startup_mute_cb(void);
void single_apa_mute(u8 mute);
void set_apap_output_status(char val);
void set_apan_output_status(char val);


#endif
