#include "includes.h"
#include "app_config.h"
#include "audio_adc.h"
#include "audio_dac.h"
#include "app_modules.h"
#if UPDATE_V2_EN
#include "code_v2/update_loader_download.h"
#endif

#if (DAC_TRACK_NUMBER == 1)
const bool config_dac_points_combine = 1;// 单声道融合左右声道数据 1:融合 0:不融合
/* 融合公式:L*Lgain/Totalgain + R*Rgain/Totalgain */
const u8 config_dac_points_Lgain = 128;
const u8 config_dac_points_Rgain = 128;
const u8 config_dac_points_Totalgain = 128;
#else
const bool config_dac_points_combine = 0;
#endif

/*************audio adc analog config************************************/

/*************audio analog config****************************************
 * 以下配置，作用为系统音频 模块开机默认配置
 * */
const bool config_vcm_cap_addon = 0;// 0 :vcm不会外挂电容；1：vcm会外挂电容；
u32 const config_audio_low_voltage_mode = 0;//0:高压 1:低压
u32 const audio_adc_mic_pga_n6db = 0;// 0:0dB  1:-6dB
AUDIO_MICPGA_G const audio_adc_mic_pga_g = AUMIC_21db;//后级增益
AUDIO_MIC_INPUT_MODE const audio_adc_mic_input_mode = mic_input_ana1_pa4;//micin输入方式

#if AMM_RS_INSIDE_ENABLE
AUDIO_MIC_RS_MODE const audio_adc_mic_rs_mode = mic_rs_inside;
#elif AMM_RS_OUTSIDE_ENABLE
AUDIO_MIC_RS_MODE const audio_adc_mic_rs_mode = mic_rs_outside;
#else
AUDIO_MIC_RS_MODE const audio_adc_mic_rs_mode = mic_rs_null;
#endif

#if AMM_DIFF_ENABLE
u32 const audio_adc_diff_mic_mode = 1;// 差分mic使能(N端固定PA6) 0:单端mic  1:差分mic
#else
u32 const audio_adc_diff_mic_mode = 0;// 差分mic使能(N端固定PA6) 0:单端mic  1:差分mic
#endif

AUDIO_MICBIAS_RS const audio_adc_mic_bias_rs = AUMIC_1k5;//micbias内部偏置电阻选择
AUDIO_MICLDO_VS const audio_adc_mic_ldo_vs = AUMIC_2v0;//micldo偏置电压选择

u32 const audio_adc_diff_aux_mode = 0;// 差分aux使能(N端固定PA6) 0:单端aux  1:差分aux
AUDIO_MICPGA_G const audio_adc_aux_pga_g = AUMIC_15db;//linein输入增益
AUDIO_MIC_INPUT_MODE const audio_adc_aux_input_mode = mic_input_ana1_pa4;//auxin输入方式

u32 const audio_adc_con1 = A1_ADC_ASYNC_IE | A1_ADC_HPSET | A1_ADC_DC_ENABLE;

//////////////////////update///////////////////////////
#define CONFIG_DOUBLE_BANK_ENABLE 0
#define OTA_TWS_SAME_TIME_NEW 0
/* #ifdef CONFIG_256K_FLASH */
/* const int config_update_mode = UPDATE_UART_EN; */
/* #else */
/* const int config_update_mode = UPDATE_BT_LMP_EN \ */
/*                                | UPDATE_STORAGE_DEV_EN | UPDATE_BLE_TEST_EN | UPDATE_UART_EN | UPDATE_UART_IO_EN; */
/* #endif */

//是否采用双备份升级方案:0-单备份;1-双备份
#if CONFIG_DOUBLE_BANK_ENABLE
const int support_dual_bank_update_en = 1;
#else
const int support_dual_bank_update_en = 0;
#endif  //CONFIG_DOUBLE_BANK_ENABLE

#if OTA_TWS_SAME_TIME_NEW       //使用新的同步升级流程
const int support_ota_tws_same_time_new =  1;
#else
const int support_ota_tws_same_time_new =  0;
#endif
//是否支持升级之后保留vm数据
const int support_vm_data_keep = 0;

//是否支持外挂flash升级,需要打开Board.h中的TCFG_NOR_FS_ENABLE
const int support_norflash_update_en  = 0;

//支持从外挂flash读取ufw文件升级使能
const int support_norflash_ufw_update_en = 0;
