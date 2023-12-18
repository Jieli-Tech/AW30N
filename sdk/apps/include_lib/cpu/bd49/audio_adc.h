#ifndef __AU_ADC_H__
#define __AU_ADC_H__

#include "typedef.h"
// #include "audio_analog.h"


#define A0_ADC_CHE           (1<<16)
#define A0_ADC_CHE_BS        16
#define A0_ADC_CHE_LEN       (19 -16 +1)

#define A0_ADC_UDE           BIT(10)
#define A0_ADC_FIFO_REST     BIT(8)
#define A0_ADC_PND           BIT(7)
#define A0_ADC_PND_CLR       BIT(6)
#define A0_ADC_IE            BIT(5)
#define A0_ADC_EN            BIT(4)


#define A1_ADC_ASYNC_PND     BIT(21)
#define A1_ADC_ASYNC_PND_CLR BIT(20)
#define A1_ADC_ASYNC_IE      BIT(19)
#define A1_ADC_HPSET         (1 << 14)
#define A1_ADC_DC_SCTL       BIT(13)
#define A1_ADC_DC_ENABLE     BIT(12)


// #define AU_ADC_HPSET_BS      12
// #define AU_ADC_HPSET_LEN     (15 - 12 + 1)
// #define AU_ADC_HPSET_DEFAULT (14 << 12)

// #define AU_ADC_SR_BS         0
// #define AU_ADC_SR            0xf
// #define AU_ADC_SR_LEN        (3 - 0 + 1)




#define AUDIO_ADC_SP_BITS     16
#define AUDIO_ADC_PACKET_SIZE (48*5)
#define AUDIO_ADC_SP_SIZE     (AUDIO_ADC_SP_BITS / 8)
#define AUDIO_ADC_SP_PNS      (AUDIO_ADC_PACKET_SIZE / 3)

#define AUDIO_ADC_CON_DEFAULT ( \
        A0_ADC_IE               | \
        A0_ADC_PND_CLR          | \
        A0_ADC_UDE              | \
        A0_ADC_CHE              \
        )

// #define auadc_cic_gain0(n)   (( n & 0x3f) << 16)
// #define auadc_cic_gain1(n)   (( n & 0x3f) << 22)


typedef struct __AUDIO_ADC_CTRL_HDL {
    void *buf;
    u16 pns;
    u16 sp_total;
    u8  sp_size;
} AUDIO_ADC_CTRL_HDL;


u32 audio_adc_clr_pnd(void);
void audio_adc_isr(void);
bool audio_adc_enable(void);
void audio_adc_disable(void);
u32 read_audio_adc_sr(void);
u32 get_audio_adc_sr(u32 sr);
void set_audio_adc_sr(u32 sr);
u32 audio_adc_phy_init(AUDIO_ADC_CTRL_HDL *p_adc, u32 sr, u32 con, u32 throw);
void audio_adc_phy_off(void);

void audio_adc_analog_init();
void audio_adc_analog_off();

void auadc_analog_variate_init(void);
#if 1
#define MACRO_MIC_RS_INSIDE     0
#define MACRO_MIC_RS_OUTSIDE    1
#define MACRO_MIC_DIFF          2
typedef enum __AUMIC_RS_MODE {
    mic_rs_inside = 0,  //PA7引脚输出MICLDO+内部偏置电阻
    mic_rs_outside = 1, //PA7引脚输出MICLDO+外部偏置电阻
    mic_rs_null = 3,    //外置电源+外置电阻
} AUDIO_MIC_RS_MODE;
extern AUDIO_MIC_RS_MODE const audio_adc_mic_rs_mode;

#define MACRO_MIC_INPUT_ANA0_PA8    0
#define MACRO_MIC_INPUT_ANA1_PA4    1
#define MACRO_MIC_INPUT_ANA2_PA5    2
#define MACRO_MIC_INPUT_ANA3_PA6    3
#define MACRO_MIC_INPUT_ANA4_PA7    4
typedef enum __AUMIC_INPUT_MODE {
    mic_input_ana0_pa8 = 0,          //PA8单端mic输入
    mic_input_ana1_pa4 = 1,          //PA4单端mic输入
    mic_input_ana2_pa5 = 2,          //PA5单端mic输入(PA5为模拟DAC输出,仅当模拟DAC关闭时可使用该I/O输入)
    mic_input_ana3_pa6 = 3,          //PA6单端mic输入(PA6为差分mic的固定N端,使用差分mic时P端输入不可选择PA6)
    mic_input_ana4_pa7 = 4,          //PA7单端mic输入(PA7为micldo/midbias输出,仅当使用外部电源+偏置时可使用该I/O输入)
} AUDIO_MIC_INPUT_MODE;
extern AUDIO_MIC_INPUT_MODE const audio_adc_mic_input_mode;

typedef enum __AUDIO_MICLDO_VS {
    AUMIC_2v0 = 0,
    AUMIC_2v2 = 1,
    AUMIC_2v4 = 2,
    AUMIC_2v6 = 3,
    AUMIC_2v7 = 4,
    AUMIC_2v8 = 5,
    AUMIC_2v9 = 6,
    AUMIC_3v0 = 7,
} AUDIO_MICLDO_VS;
extern AUDIO_MICLDO_VS const audio_adc_mic_ldo_vs;

typedef enum __AUDIO_MICBIAS_RS {
    AUMIC_0k5 = 0,
    AUMIC_1k0 = 1,
    AUMIC_1k5 = 2,
    AUMIC_2k0 = 3,
    AUMIC_2k5 = 4,
    AUMIC_3k0 = 5,
    AUMIC_3k5 = 6,
    AUMIC_4k0 = 7,
    AUMIC_4k5 = 8,
    AUMIC_5k0 = 9,
    AUMIC_6k0 = 0xa,
    AUMIC_7k0 = 0xb,
    AUMIC_8k0 = 0xc,
    AUMIC_9k0 = 0xd,
    AUMIC_10k = 0xe,
    AUMIC_RS_READ = 0xf,
} AUDIO_MICBIAS_RS;
extern AUDIO_MICBIAS_RS const audio_adc_mic_bias_rs;

// typedef enum __AUDIO_MICIN_PORT {
//     AUMIC_NULL = 0,
//     AUMIC_N = 1,
//     AUMIC_P = 2,
//     // AUMIC_MUTE = 3,
// } AUDIO_MICIN_PORT;

// extern u32 const audio_adc_mic_bias_2_vcmo0;
// extern u32 const audio_adc_mic_bias_2_vcmo1;

typedef enum __AUDIO_MICPGA_G {
    // 前后级增益档位说明
    // audio_adc_mic_pga_n6db = 1时，audio_adc_mic_pga_g可选0dB、3dB、9dB;
    // audio_adc_mic_pga_n6db = 0时，audio_adc_mic_pga_g可选0dB、3dB、9dB、15dB、21dB;
    AUMIC_0db = 0,
    AUMIC_3db = 1,
    AUMIC_9db = 2, //前级-6dB(n6db)增益使能时,最高只可配置AUMIC_9db,后续档位无效
    AUMIC_15db = 3,
    AUMIC_21db = 4,
} AUDIO_MICPGA_G;

u32 set_auadc_mic_bias_rs(AUDIO_MICBIAS_RS bias_rs);
void set_auadc_micldo_vs(AUDIO_MICLDO_VS ldo_vs);
// void set_auadc_micin_pa1(AUDIO_MICIN_PORT pa1);
// void set_auadc_micin_pa2(AUDIO_MICIN_PORT pa2);
// void set_auadc_mic_bias_2_vcom0(bool flag);
// void set_auadc_mic_bias_2_vcom1(bool flag);
void set_auadc_mic_pga_n6db(bool enable);
void set_auadc_mic_pga(AUDIO_MICPGA_G pga);
void set_auadc_aux_pga(AUDIO_MICPGA_G pga);

#endif


#endif
