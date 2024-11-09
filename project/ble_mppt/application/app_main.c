#pragma bss_seg(".app.data.bss")
#pragma data_seg(".app.data")
#pragma const_seg(".app.text.const")
#pragma code_seg(".app.text")
#pragma str_literal_override(".app.text.const")

#include "includes.h"

#include "mcpwm.h"
#include "adc_api.h"
#include "gpio.h"
#include "hw_timer.h"

#define LOG_TAG_CONST       APP
#define LOG_TAG             "[ble mppt]"
#include "log.h"

void m_gpio_test(void);
void m_pwm_test(void);
void m_pwm_set_duty(uint32_t duty);
void m_adc_test(void);
void m_adc_loop(void);

static void m_adc_timer_time_cb(void);
static void m_adc_timer_init();

AT(.tick_timer.text.cache.L2)
void tick_timer_ram_loop(void)
{

}

void app_timer_loop(void)
{
    extern u8 tick_cnt;
    tick_cnt++;
}

void app_main(void)
{
    log_info("BLE MPPT App Start\n");
    extern u32 get_up_suc_flag();
    if (get_up_suc_flag()) {
        log_info("----device update end----\n");
        wdt_close();
        while (1);
    }

    vm_isr_response_index_register(IRQ_TICKTMR_IDX);//vm擦写flash时响应tick_timer_ram_loop()函数
    delay_10ms(50);//等待系统稳定

    uint8_t work_mode = 0;
    sysmem_read_api(SYSMEM_INDEX_SYSMODE, &work_mode, sizeof(work_mode));
    while (1) {
        wdt_clear();
        clear_all_message();
        os_time_dly(1);
    }
}

void app(void)
{
    app_main();

    log_error("No app running!!!\n");
    while (1) {
        wdt_clear();
    }
}

typedef enum{
    CALIB_2V5_IDX = 0,
    SOLAR_IN_I_IDX,
    SOLAR_IN_V_IDX,
    SOLAR_OUT_I_IDX,
    SOLAR_OUT_V1_IDX,
    SOLAR_OUT_V2_IDX,
    SOLAR_ADC_CH_NUM,
}solar_adc_ch_t;

struct adc_handle_t {
    uint32_t io;
    uint32_t channel;
    uint32_t value;
};

typedef struct __solar_board_t
{
    struct adc_handle_t adc_handle[SOLAR_ADC_CH_NUM];

    int  adc_3v3_calib_factor;

    int  solar_in_volt;
    int  solar_in_curr;
    int  solar_in_i_offset;

    int  solar_out_curr;
    int  solar_out_volt1;
    int  solar_out_volt2;
    int  solar_out_i_offset;
}solar_board_t;

/**
计算空载输出电压 Vout
I总 = I1+ I2

Vout - Vfb           Vfb               Vfb - Vda
—————— = ————————— + ———————————
  R1(430K)     R2(100K)并R3(2K)         R4(36K)
  ==============================>
Vout = 56.9861 - Vda*11.9444   (可调范围 56.986-(0 ~ 39.416))
*/

static JL_TIMER_TypeDef *TIMER_PWM = JL_TIMER2;
static JL_TIMER_TypeDef *TIMER_ADC = JL_TIMER0;

static solar_board_t solar_board =
{
    .adc_handle[CALIB_2V5_IDX].io      = IO_PORTA_04,
    .adc_handle[SOLAR_IN_I_IDX].io     = IO_PORTA_08,
    .adc_handle[SOLAR_IN_V_IDX].io     = IO_PORTA_05,
    .adc_handle[SOLAR_OUT_I_IDX].io    = IO_PORTA_13,
    .adc_handle[SOLAR_OUT_V1_IDX].io   = IO_PORTA_14,
    .adc_handle[SOLAR_OUT_V2_IDX].io   = IO_PORTA_15,
};

void m_gpio_test(void)
{
    struct gpio_config config;

    config.pin  = PORT_PIN_6 | PORT_PIN_7;
    config.mode = PORT_OUTPUT_LOW;
    config.hd   = PORT_DRIVE_STRENGT_24p0mA;
    gpio_init(PORTA, &config);

    config.pin  = PORT_PIN_2 | PORT_PIN_3 | PORT_PIN_4;
    gpio_init(PORTB, &config);

    gpio_write(IO_PORTB_02, 1);
    gpio_write(IO_PORTB_03, 1);
    gpio_write(IO_PORTB_04, 1);
}

void m_pwm_test(void)
{
    gpio_set_mode(IO_PORT_SPILT(IO_PORTB_01), PORT_OUTPUT_LOW);
    hw_timer_pwm_init(TIMER_PWM, IO_PORTB_01, 100000, 5000); // 频率100K, 占空比50%
    hw_timer_pwm_set_duty(TIMER_PWM, 5000);
}

void m_pwm_set_duty(uint32_t duty)
{
    hw_timer_pwm_set_duty(TIMER_PWM, duty);
}

static void m_adc_timer_time_cb(void)
{
    if (TIMER_ADC->CON &TIMER_PND) {
        TIMER_ADC->CON |= TIMER_PCLR;
        m_adc_loop();
    }
}

static void m_adc_timer_init()
{
    TIMER_ADC = JL_TIMER0;
    hw_timer_init(TIMER_ADC, 1); //定时器初始化，100ms
    request_irq(IRQ_TIME0_IDX, 1, m_adc_timer_time_cb, 0);    //设置定时器中断(中断号，优先级，回调函数，cpu核)
}

void m_adc_test(void)
{
    adc_init();

    for(uint8_t i=0; i<SOLAR_ADC_CH_NUM; i++)
    {
        solar_board.adc_handle[i].channel = adc_io2ch(solar_board.adc_handle[i].io);
        adc_add_sample_ch(solar_board.adc_handle[i].channel);
        adc_set_sample_period(solar_board.adc_handle[i].channel, 1);
    }

    m_adc_timer_init();
}

void m_adc_loop(void)
{
    static uint8_t init_flag = 0;
    static int in_i_offset = 0;
    static int in_o_offset = 0;
    static int offset_cnt = 0;

    uint32_t ad_value = 0;
    for(uint8_t i=0; i<SOLAR_ADC_CH_NUM; i++)
    {
        solar_board.adc_handle[i].value = adc_get_value(solar_board.adc_handle[i].channel);
    }

    adc_sample(solar_board.adc_handle[CALIB_2V5_IDX].channel, 1);

    if(!init_flag)
    {
        offset_cnt++;

        if(offset_cnt >= 300)
        {
            init_flag = 1;
            solar_board.solar_in_i_offset  = in_i_offset / 100;
            solar_board.solar_out_i_offset = in_i_offset / 100;
            in_o_offset = in_o_offset / 100;
            offset_cnt = 0;
        }
        else if(offset_cnt >= 200)
        {
            in_i_offset  += solar_board.adc_handle[SOLAR_IN_I_IDX].value;
            in_o_offset  += solar_board.adc_handle[SOLAR_OUT_I_IDX].value;
        }
    }
    else
    {
        // 根据2.5V 校准系数
        solar_board.adc_3v3_calib_factor = (2.5f / (solar_board.adc_handle[CALIB_2V5_IDX].value / 4096.0f * 3.3f)) * 1000;

        // 输入电流电压计算
        solar_board.solar_in_curr   = ((solar_board.adc_handle[SOLAR_IN_I_IDX].value - solar_board.solar_in_i_offset) / 4096.0f * 3.3f)   * solar_board.adc_3v3_calib_factor / 50 /0.005f;
        solar_board.solar_in_volt   = (solar_board.adc_handle[SOLAR_IN_V_IDX].value / 4096.0f * 3.3f)   * solar_board.adc_3v3_calib_factor * 10;

        // 输出电流电压计算
        solar_board.solar_out_curr  = ((solar_board.adc_handle[SOLAR_OUT_I_IDX].value - solar_board.solar_out_i_offset) / 4096.0f * 3.3f)  * solar_board.adc_3v3_calib_factor / 50 /0.005f;
        solar_board.solar_out_volt1 = (solar_board.adc_handle[SOLAR_OUT_V1_IDX].value / 4096.0f * 3.3f) * solar_board.adc_3v3_calib_factor * 50;
        solar_board.solar_out_volt2 = (solar_board.adc_handle[SOLAR_OUT_V2_IDX].value / 4096.0f * 3.3f) * solar_board.adc_3v3_calib_factor * 50;
    }
}

