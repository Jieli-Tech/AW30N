/***********************************Jieli tech************************************************
  File : adc_api.c
  By   : liujie
  Email: liujie@zh-jieli.com
  date : 2019-1-14
********************************************************************************************/
#include "uart.h"
#include "config.h"
#include "audio.h"
#include "audio_adc.h"
#include "audio_adc_api.h"
#include "audio_dac.h"


#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[normal]"
#include "log.h"

short adc_isr_ext_buf[AUDIO_ADC_SP_PNS];//audio_adc驱动使用，不可修改
extern u32 const audio_adc_diff_mic_mode;
extern AUDIO_MIC_RS_MODE const audio_adc_mic_rs_mode;
extern AUDIO_MIC_INPUT_MODE const audio_adc_mic_input_mode;
extern u32 const audio_adc_diff_aux_mode;
extern AUDIO_MIC_INPUT_MODE const audio_adc_aux_input_mode;

#define SET_IO_2_ANALOG_STATE(port,num) \
            JL_PORT##port->DIR |=  BIT(num);  \
            JL_PORT##port->DIE &= ~BIT(num);  \
            JL_PORT##port->PU0 &= ~BIT(num);  \
            JL_PORT##port->PU1 &= ~BIT(num);  \
            JL_PORT##port->PD0 &= ~BIT(num);  \
            JL_PORT##port->PD1 &= ~BIT(num);

static bool auadc_mutex_check(void)
{
    /* 差分mic N端固定占用PA6，使能差分mic时mic P端不可选择PA6输入 */
    if (audio_adc_diff_mic_mode && (mic_input_ana3_pa6 == audio_adc_mic_input_mode)) {
        log_error("ADC diff micin_p can't select to PA6 while micin_n defaultly using PA6!");
        return false;
    }
    /* 模拟DAC输出固定占用PA5，使用期间micin不可选择PA5输入 */
    if ((AUOUT_USE_DAC == AUOUT_USE_RDAC) && (mic_input_ana2_pa5 == audio_adc_mic_input_mode)) {
        log_error("ADC micin can't select to PA5 while DAC defaultly using PA5!");
        return false;
    }

    return true;
}

void auadc_open_mic(void)
{
    if (false == audio_clk_open_check()) {
        return;
    }

    if (false == auadc_mutex_check()) {
        return;
    }

    single_micin_analog_open();

    if (mic_rs_null != audio_adc_mic_rs_mode) {
        SET_IO_2_ANALOG_STATE(A, 7);
    }
    if (audio_adc_diff_mic_mode) {
        /* 差分micin */
        JL_ADDA->ADA_CON0 |= BIT(23);
        SET_IO_2_ANALOG_STATE(A, 6);
    } else {
        /* 单端micin */
        JL_ADDA->ADA_CON0 |= BIT(20);
    }

    JL_ADDA->ADA_CON0 &= ~(0x1f << 10);
    switch (audio_adc_mic_input_mode) {
    case mic_input_ana0_pa8:
        JL_ADDA->ADA_CON0 |= BIT(10);
        SET_IO_2_ANALOG_STATE(A, 8);
        break;
    case mic_input_ana1_pa4:
        JL_ADDA->ADA_CON0 |= BIT(11);
        SET_IO_2_ANALOG_STATE(A, 4);
        break;
    case mic_input_ana2_pa5:
        JL_ADDA->ADA_CON0 |= BIT(12);
        SET_IO_2_ANALOG_STATE(A, 5);
        break;
    case mic_input_ana3_pa6:
        JL_ADDA->ADA_CON0 |= BIT(13);
        SET_IO_2_ANALOG_STATE(A, 6);
        break;
    case mic_input_ana4_pa7:
        JL_ADDA->ADA_CON0 |= BIT(14);
        SET_IO_2_ANALOG_STATE(A, 7);
        break;
    }
}

void auadc_open_linein(void)
{
    if (false == audio_clk_open_check()) {
        return;
    }
    if (audio_adc_diff_aux_mode && \
        (mic_input_ana3_pa6 == audio_adc_aux_input_mode)) {
        log_error("ADC diff auxin_p can't select to PA6 while auxin_n defaultly using PA6!");
        return;
    }
    single_linin_analog_open();
    if (audio_adc_diff_aux_mode) {
        /* 差分auxin */
        JL_ADDA->ADA_CON0 |= BIT(23);
        SET_IO_2_ANALOG_STATE(A, 6);
    } else {
        /* 单端auxin */
        JL_ADDA->ADA_CON0 |= BIT(20);
    }

    JL_ADDA->ADA_CON0 &= ~(0x1f << 10);
    switch (audio_adc_aux_input_mode) {
    case mic_input_ana0_pa8:
        JL_ADDA->ADA_CON0 |= BIT(10);
        SET_IO_2_ANALOG_STATE(A, 8);
        break;
    case mic_input_ana1_pa4:
        JL_ADDA->ADA_CON0 |= BIT(11);
        SET_IO_2_ANALOG_STATE(A, 4);
        break;
    case mic_input_ana2_pa5:
        JL_ADDA->ADA_CON0 |= BIT(12);
        SET_IO_2_ANALOG_STATE(A, 5);
        break;
    case mic_input_ana3_pa6:
        JL_ADDA->ADA_CON0 |= BIT(13);
        SET_IO_2_ANALOG_STATE(A, 6);
        break;
    case mic_input_ana4_pa7:
        JL_ADDA->ADA_CON0 |= BIT(14);
        SET_IO_2_ANALOG_STATE(A, 7);
        break;
    }
}








