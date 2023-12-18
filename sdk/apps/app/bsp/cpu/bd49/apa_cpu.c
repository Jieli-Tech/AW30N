
/***********************************Jieli tech************************************************
  File : dac_api.c
  By   : liujie
  Email: liujie@zh-jieli.com
  date : 2019-1-14
********************************************************************************************/

#pragma bss_seg(".audio_dac_cpu.data.bss")
#pragma data_seg(".audio_dac_cpu.data")
#pragma const_seg(".audio_dac_cpu.text.const")
#pragma code_seg(".audio_dac_cpu.text")
#pragma str_literal_override(".audio_dac_cpu.text.const")

#include "audio_dac.h"
#include "audio_dac_api.h"
#include "audio.h"
#include "audio_analog.h"
#include "app_config.h"

#define LOG_TAG_CONST       NORM
/* #define LOG_TAG_CONST       OFF */
#define LOG_TAG             "[audac_cpu]"
#include "log.h"


/* *******************
 * audio dac 功夫接线方式
 *
 * */
u32 apa_track_number(void)
{
    return 1;
    /* if (A_DAC_STERO == AUDAC_OUTPUT_MODE) { */
    /* return 2; */
    /* } else { */
    /* return 1; */
    /* } */
}

u16 apa_dual_buf[2][DAC_PACKET_SIZE];
/* #define DAC_MAX_SP  (sizeof(apa_dual_buf) / (DAC_TRACK_NUMBER * (AUDAC_BIT_WIDE / 8))) */

/* #define DAC_PNS     (DAC_MAX_SP/3) */
/* #define sound_out_obj void */
_OTP_CONST_  APA_CTRL_HDL apa_ops = {
    .buf0   = &apa_dual_buf[0][0],
    .buf1   = &apa_dual_buf[1][0],
    .sp_len = sizeof(apa_dual_buf) / 2,
    .con0    = DAC_DEFAULT,
};

void apa_clr_buf(void)
{
    memset(&apa_dual_buf[0][0], 0, sizeof(apa_dual_buf));
}

void apa_trim_api(void)
{
}
void apa_analog_open_api(u32 delay_flag)
{
    apa_analog_open();
}
void test_audio_apa(void)
{
    log_info("JL_CLOCK->PRP_CON0 : 0x%x", JL_CLOCK->PRP_CON0);
    log_info("JL_APA->APA_CON0   : 0x%x", JL_APA->APA_CON0);
    log_info("JL_APA->APA_CON1   : 0x%x", JL_APA->APA_CON1);
    log_info("JL_APA->APA_CON2   : 0x%x", JL_APA->APA_CON2);
    log_info("JL_APA->APA_CON3   : 0x%x", JL_APA->APA_CON3);
}

void apa_phy_vol(u16 dac_l, u16 dac_r)
{
}



void apa_cpu_mode(void)
{
}

/* int dac_low_power(void) */
/* { */
/* return 0; */
/* } */

u32 apa_mode_check(u32 con)
{
    return con;
}
/* #define APA_HALF_PARA 1 */
#define APA_HALF_PARA 3

/************START apa_para_define**********************************************/
#define apasr_con1(n10, n0, n8)   ((n10 << 10)| (n8 << 8) | (n0 << 0))

#if (APA_PLL240M == APA_CLK_DEFAULT)
/*---------------START apa_240M_para_define--------------------*/

#define  CON0_48K   (( 624 << 22) | (1<<18) | (3<<16))
#define  CON0_44K1  (( 679 << 22) | (1<<18) | (3<<16))
#define  CON0_32K   (( 624 << 22) | (2<<18) | (3<<16))

#if (APA_MODE1_NOR == APA_MODE_DEFAULT)
#define CON1_48K    apasr_con1(156, 69, 2)
#define CON1_44K1   apasr_con1(170, 75, 2)
#define CON1_32K    apasr_con1(156, 69, 2)
#elif (APA_MODE2_SNR == APA_MODE_DEFAULT)
#define CON1_48K    apasr_con1(312, 69, 3)
#define CON1_44K1   apasr_con1(340, 75, 3)
#define CON1_32K    apasr_con1(312, 69, 3)
#elif (APA_MODE3_THD == APA_MODE_DEFAULT)
#define CON1_48K    apasr_con1(156, 69, 2)
#define CON1_44K1   apasr_con1(170, 75, 2)
#define CON1_32K    apasr_con1(156, 69, 2)
#endif
/*---------------END apa_240M_para_define--------------------*/

#elif (APA_PLL320M == APA_CLK_DEFAULT)
/*---------------START apa_320M_para_define--------------------*/

#define  CON0_48K      (( 416 << 22) | (APA_HALF_PARA<<18) | (3<<16))
#define  CON0_44K1     (( 453 << 22) | (APA_HALF_PARA<<18) | (3<<16))
#define  CON0_32K      (( 624 << 22) | (3<<18) | (3<<16))


#if (APA_MODE1_NOR == APA_MODE_DEFAULT)
#define CON1_48K    apasr_con1(104,  92, 1)
#define CON1_44K1   apasr_con1(113, 100, 1)
#define CON1_32K    apasr_con1(156,  69, 2)
#elif (APA_MODE2_SNR == APA_MODE_DEFAULT)
#define CON1_48K    apasr_con1(208,  92, 2)
#define CON1_44K1   apasr_con1(227, 101, 2)
#define CON1_32K    apasr_con1(312,  69, 3)
#elif (APA_MODE3_THD == APA_MODE_DEFAULT)
#define CON1_48K    apasr_con1(104,  92, 1)
#define CON1_44K1   apasr_con1(113, 100, 1)
#define CON1_32K    apasr_con1(156,  69, 2)
#endif
/*---------------END apa_320M_para_define--------------------*/

#endif/* END #if (APA_PLL240M == APA_CLK_DEFAULT)*/

enum {
    DCC_238HZ4 = 8,
    DCC_119HZ4,
    DCC_59HZ7,
    DCC_29HZ7,
    DCC_14HZ8,
    DCC_7HZ5,
    DCC_3HZ6,
    DCC_1HZ8,
} ;
#define CON2_APA ( (0<<18) | (0 << 17) | (1 << 16) | (0 << 0) )

/************END apa_para_define**********************************************/
const u32 sr48k_para[3] = {
    CON0_48K,  CON1_48K | (DCC_3HZ6 << 24), CON2_APA
};

const u32 sr44k1_para[3] = {
    CON0_44K1, CON1_44K1 | (DCC_3HZ6 << 24), CON2_APA
};

const u32 sr32k_para[3] = {
    CON0_32K,  CON1_32K | (DCC_3HZ6 << 24), CON2_APA
};

const u32 *apa_get_para(u32 con)
{
    u32 sr_mode = con & APA_SRBITS;
    switch (sr_mode) {
    case APA_SR32K:
        return &sr32k_para[0];
    case APA_SR44K1:
        return &sr44k1_para[0];
    case APA_SR48K:
        return &sr48k_para[0];
    default:
        return &sr44k1_para[0];
    }
}
void apa_mode_init(void)
{
    apa_clr_buf();
    apa_resource_init((void *)&apa_ops);
}

#include "clock.h"
void apa_init(u32 sr, u32 delay_flag)
{
    apa_phy_init(apa_sr_lookup(sr));
    if (delay_flag) {
        udelay(2000);
    }
    apa_trim_api();
    apa_analog_open_api(delay_flag);
}

void apa_off_api(void)
{
    apa_phy_off();
    apa_analog_close();
}

void apa_sr_api(u32 sr)
{
    /* u32 dac_sr_set(u32 sr) */
    apa_sr_set(apa_sr_lookup(sr));
    /* dac_analog_init(); */
}

#define APA_NORMAL_OUT      0
#define APAP_OUT_APAN_MUTE  1
#define APAN_OUT_APAP_MUTE  2
#if (0 == (DAC_DEFAULT & APA_MODE2_SNR))
//AD17N mode1模式单端推功放功能配置
//0：APAP / APAN均输出信号，差分推功放
//1：APAP输出信号，APAN作普通IO控制功放MUTE
//2：APAN输出信号，APAP作普通IO控制功放MUTE
#define SINGLE_APA_ENABLE       APA_NORMAL_OUT
//APA单端输出时，开机MUTE脚状态配置，需根据功放MUTE电平配置：
//0：MUTE脚开机输出低电平
//1：MUTE脚开机输出高电平
#define SINGLE_APA_MUTE_VALUE   1
#else
#define SINGLE_APA_ENABLE       APA_NORMAL_OUT
#define SINGLE_APA_MUTE_STATUS  0
#endif

/* audio库调用，初始化APA时MUTE住防止po声 */
void single_apa_startup_mute_cb(void)
{
#if (SINGLE_APA_ENABLE == APAP_OUT_APAN_MUTE)
    set_apan_output_status(SINGLE_APA_MUTE_VALUE);
#elif (SINGLE_APA_ENABLE == APAN_OUT_APAP_MUTE)
    set_apap_output_status(SINGLE_APA_MUTE_VALUE);
#endif
}

void single_apa_mute(u8 mute)
{
#if (SINGLE_APA_ENABLE == APAP_OUT_APAN_MUTE)
    if (mute) {
        set_apan_output_status(SINGLE_APA_MUTE_VALUE);
    } else {
        set_apan_output_status(!SINGLE_APA_MUTE_VALUE);
    }
#elif (SINGLE_APA_ENABLE == APAN_OUT_APAP_MUTE)
    if (mute) {
        set_apap_output_status(SINGLE_APA_MUTE_VALUE);
    } else {
        set_apap_output_status(!SINGLE_APA_MUTE_VALUE);
    }
#endif
}




/* void test_apa(void) */
/* { */
/* audio_init(); */
/* dac_phy_vol(16384, 16384); */
/* dac_mode_init(31); */
/* dac_init_api(32000); */
/* } */
