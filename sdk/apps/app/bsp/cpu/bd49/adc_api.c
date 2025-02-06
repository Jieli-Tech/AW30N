#include "typedef.h"
#include "clock.h"
#include "adc_api.h"
#include "gpio.h"
/* #include "timer.h" */
#include "init.h"
/* #include "efuse.h" */
#include "irq.h"
#include "power_interface.h"
#include "power/power_manage.h"
#include "jiffies.h"
/* #include "hw_timer.h" */

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[saradc]"
#include "log.h"

u8 cur_ch = ADC_MAX_CH;  //adc采集队列当前编号
u8 adc_clk_div = 0; //adc时钟分频系数

struct adc_info_t adc_queue[ADC_MAX_CH + ENABLE_PREEMPTIVE_MODE];  //采集队列声明

static u8 adc_ch_io_table[8] = {  //gpio->adc_ch 表
    IO_PORTA_04,
    IO_PORTA_05,
    IO_PORTA_08,
    IO_PORTA_13,
    IO_PORTA_14,
    IO_PORTA_15,
    IO_PORT_DP,
    IO_PORT_DM,
};

static void clock_critical_enter(void)
{
}
static void adc_adjust_div(void)
{
    adc_clk_div = 0x7f; //adc_clk_div 占7bit
    const u32 lsb_clk = clk_get("lsb");
    log_info("lsb %d\n", lsb_clk);
    for (int i = 1; i < 0x80; i++) {
        if (((lsb_clk / i) / 2) <= 12000000) {
            adc_clk_div = i;
            break;
        }
    }
}
LSB_CRITICAL_HANDLE_REG(saradc, clock_critical_enter, adc_adjust_div)


u32 adc_io2ch(int gpio)   //根据传入的GPIO，返回对应的ADC_CH
{
    for (u8 i = 0; i < ARRAY_SIZE(adc_ch_io_table); i++) {
        if (adc_ch_io_table[i] == gpio) {
            return (ADC_CH_TYPE_IO | i);
        }
    }
    log_error("add_adc_ch io error!!! change other io_port!!!\n");
    return 0xffffffff; //未找到支持ADC的IO
}

void adc_io_ch_set(enum AD_CH ch, u32 mode) //adc io通道 模式设置
{

}

void adc_internal_signal_to_io(u32 analog_ch, u16 adc_io) //将内部通道信号，接到IO口上，输出
{
    gpio_set_mode(IO_PORT_SPILT(adc_io), PORT_HIGHZ);

    adc_sample(analog_ch, 0);
    u32 ch = adc_io2ch(adc_io);
    u16 adc_ch_sel = ch & ADC_CH_MASK_CH_SEL;
    SFR(JL_ADC->ADC_CON, 6, 4, adc_ch_sel);
    SFR(JL_ADC->ADC_CON, 3, 3, 0b011); //cpu adc io sel en
}
static void _adc_pmu_vbg_enable()
{
    p33_or_1byte(P3_PMU_ADC0, BIT(4));
    udelay(11);
    udelay(11);
    udelay(11);
    udelay(11);
    udelay(11);
    p33_or_1byte(P3_PMU_ADC0, BIT(5));
}
static void _adc_pmu_vbg_diable()
{
    p33_and_1byte(P3_PMU_ADC0, (u8)~BIT(5));
    udelay(11);
    udelay(11);
    udelay(11);
    udelay(11);
    udelay(11);
    p33_and_1byte(P3_PMU_ADC0, (u8)~BIT(4));
}
static void _adc_pmu_ch_select(u16 ch)
{
    if (ch == 0) {
        p33_or_1byte(P3_PMU_ADC0, 0xc0);
    } else {
    }
    P33_CON_SET(P3_PMU_ADC0, 0, 4, ch);
    p33_or_1byte(P3_PMU_ADC1, 0b11);
}

u32 adc_add_sample_ch(enum AD_CH ch)   //添加一个指定的通道到采集队列
{
    u32 adc_type_sel = ch & ADC_CH_MASK_TYPE_SEL;
    u16 adc_ch_sel = ch & ADC_CH_MASK_CH_SEL;
    log_info("type = %x,ch = %x\n", adc_type_sel, adc_ch_sel);
    u32 i = 0;
    for (i = 0; i < ADC_MAX_CH; i++) {
        if (adc_queue[i].ch == ch) {
            break;
        } else if (adc_queue[i].ch == -1) {
            adc_queue[i].ch = ch;
            adc_queue[i].v.value = (u16) - 1;
            adc_queue[i].sample_period = 0;
#if AD_CH_IO_VBAT_PORT
            if (ch == AD_CH_IO_VBAT_PORT) {
                adc_queue[i].adc_voltage_mode = 1;
                continue;
            }
#endif
            switch (ch) {
            case AD_CH_LDOREF:
                adc_queue[i].adc_voltage_mode = 0;
                break;
            case AD_CH_PMU_VBAT:
                adc_queue[i].adc_voltage_mode = 1;
                break;
            case AD_CH_PMU_VTEMP:
                adc_queue[i].adc_voltage_mode = 1;
                break;
            default:
                adc_queue[i].adc_voltage_mode = 0;
                break;
            }
            break;
        }
    }
    return i;
}

u32 adc_delete_ch(enum AD_CH ch)    //将一个指定的通道从采集队列中删除
{
    u32 i = 0;
    for (i = 0; i < ADC_MAX_CH; i++) {
        if (adc_queue[i].ch == ch) {
            adc_queue[i].ch = -1;
            break;
        }
    }
    return i;
}

void adc_sample(enum AD_CH ch, u32 ie) //启动一次cpu模式的adc采样
{
    u32 adc_con = 0;
    u32 adc_type_sel = ch & ADC_CH_MASK_TYPE_SEL;
    u16 adc_ch_sel = ch & ADC_CH_MASK_CH_SEL;
    SFR(adc_con, 31, 1, 1);//ctl_en
    SFR(adc_con, 28, 1, 1);//clr pnd
    SFR(adc_con, 25, 1, 1);//adc_aen
    SFR(adc_con, 0, 1, 1);//adc_aen
    /* SFR(adc_con, 2, 1, 1);//CAL_KST */
    SFR(adc_con, 23, 2, 3);//12bit
    SFR(adc_con, 16, 7, adc_clk_div);//adc_clk 分频
    if (ie) {
        SFR(adc_con, 13, 1, 1);//ie
    }
    SFR(adc_con, 10, 3, adc_type_sel >> 16);//test sel
    SFR(adc_con, 3, 3, 0b001);//cpu adc test sel en

    switch (adc_type_sel) {
    case ADC_CH_TYPE_BT:
        break;
    case ADC_CH_TYPE_PMU:
        _adc_pmu_ch_select(adc_ch_sel);
        break;
    case ADC_CH_TYPE_LPCTM:
        break;
    case ADC_CH_TYPE_AUDIO:
        /* adc_audio_ch_select(adc_ch_sel); */
        break;
    case ADC_CH_TYPE_X32K:
        break;
    case ADC_CH_TYPE_DIFF:
        SFR(adc_con, 3, 3, 0b100); //cpu adc diff sel en
        break;
    default:
        SFR(adc_con, 6, 4, adc_ch_sel);
        SFR(adc_con, 3, 3, 0b010); //cpu adc io sel en
        break;
    }

    if (ch == ADC_CH_IO_DP) {
        gpio_set_mode(IO_PORT_SPILT(IO_PORT_DP), PORT_HIGHZ);
        JL_PORTUSB->CON = 0;
        JL_PORTUSB->CON |= (BIT(8) | BIT(1) | BIT(0));
    }
    if (ch == ADC_CH_IO_DM) {
        gpio_set_mode(IO_PORT_SPILT(IO_PORT_DM), PORT_HIGHZ);
        JL_PORTUSB->CON = 0;
        JL_PORTUSB->CON |= (BIT(8) | BIT(3) | BIT(0));
    }

    JL_ADC->ADC_CON = adc_con;
    if ((adc_type_sel == ADC_CH_TYPE_PMU) || (adc_type_sel == ADC_CH_TYPE_LPCTM)) {
        udelay(11); //10us
    }
    JL_ADC->ADC_CON |= BIT(2);//CAL_KST
    JL_ADC->ADC_CON |= BIT(14);//kistart
}

static void adc_wait_idle_timeout()
{
    u32 time_out_us = 1000;
    while (time_out_us--) {
        asm("csync");
        if (JL_ADC->ADC_CON & BIT(15)) {
            return;
        }
        if ((JL_ADC->ADC_CON & BIT(31)) == 0) {
            return;
        }
        udelay(1);
    }
    adc_close();
}
void adc_wait_enter_idle() //等待adc进入空闲状态，才可进行阻塞式采集
{
    if (JL_ADC->ADC_CON & BIT(15)) {
        return;
    } else {
        adc_wait_idle_timeout();
    }
}
void adc_set_enter_idle() //设置 adc cpu模式为空闲状态
{
    /* JL_ADC->ADC_CON &= ~BIT(4); */
}
u16 adc_wait_pnd()   //cpu采集等待pnd
{
    adc_wait_idle_timeout();
    u32 adc_res = JL_ADC->RES;
    adc_close();
    return adc_res;
}

void adc_close()     //adc close
{
    JL_ADC->ADC_CON |= BIT(28);//cpnd
    /* JL_ADC->ADC_CON = BIT(28);//cpnd */
}

___interrupt
static void adc_isr()   //中断函数
{
    if (adc_queue[ADC_MAX_CH].ch != -1) {
        adc_close();
        return;
    }

    if (JL_ADC->ADC_CON & BIT(29)) {
        JL_ADC->ADC_CON |= BIT(28);
        u32 adc_value = JL_ADC->RES;

        if (adc_cpu_mode_process(adc_value)) {
            return;
        }
        cur_ch++;
        cur_ch = adc_get_next_ch();
        if (cur_ch == ADC_MAX_CH) {
            adc_close();
        } else {
            adc_sample(adc_queue[cur_ch].ch, 1);
        }
    }
}

/* ___interrupt */
void adc_scan()  //定时函数，每 x ms启动一轮cpu模式采集
{
    /* if (JL_TIMER0->CON & TIMER_PND) {   //有PND */
    /*     JL_TIMER0->CON |= TIMER_PCLR;   //清PND */
    /* } */
    if (adc_queue[ADC_MAX_CH].ch != -1) {
        return;
    }
    if (!(JL_ADC->ADC_CON & BIT(15))) {
        return;
    }
    cur_ch = adc_get_next_ch();
    if (cur_ch == ADC_MAX_CH) {
        return;
    }
    adc_sample(adc_queue[cur_ch].ch, 1);
}

void adc_hw_init(void)    //adc初始化子函数
{
    memset(adc_queue, 0xff, sizeof(adc_queue));

    adc_close();

    adc_adjust_div();

    adc_add_sample_ch(AD_CH_LDOREF);
    adc_set_sample_period(AD_CH_LDOREF, PMU_CH_SAMPLE_PERIOD);

    adc_add_sample_ch(AD_CH_PMU_VBAT);
    adc_set_sample_period(AD_CH_PMU_VBAT, PMU_CH_SAMPLE_PERIOD);

    adc_add_sample_ch(AD_CH_PMU_VTEMP);
    adc_set_sample_period(AD_CH_PMU_VTEMP, PMU_CH_SAMPLE_PERIOD);

#if AD_CH_IO_VBAT_PORT
    adc_add_sample_ch(adc_io2ch(AD_CH_IO_VBAT_PORT));
    adc_set_sample_period(adc_io2ch(AD_CH_IO_VBAT_PORT), PMU_CH_SAMPLE_PERIOD);
#endif

    u32 vbg_adc_value = 0;
    u32 vbg_min_value = -1;
    u32 vbg_max_value = 0;

    for (int i = 0; i < AD_CH_PMU_VBG_TRIM_NUM; i++) {
        u32 adc_value = adc_get_value_blocking(AD_CH_LDOREF);
        /* u32 adc_value = adc_get_value_blocking(AD_CH_IO_PA1); */
        /* printf("vbg[%d] = %d\n", i, adc_value); */
        if (adc_value > vbg_max_value) {
            vbg_max_value = adc_value;
        } else if (adc_value < vbg_min_value) {
            vbg_min_value = adc_value;
        }
        vbg_adc_value += adc_value;
    }
    vbg_adc_value -= vbg_max_value;
    vbg_adc_value -= vbg_min_value;

    vbg_adc_value /= (AD_CH_PMU_VBG_TRIM_NUM - 2);
    adc_queue[0].v.value = vbg_adc_value;
    log_info("LDOREF = %d\n", vbg_adc_value);

    u32 voltage = adc_get_voltage_blocking(AD_CH_PMU_VBAT);
    adc_queue[1].v.voltage = voltage;
    log_info("vbat = %d mv\n", adc_get_voltage(AD_CH_PMU_VBAT) * 4);

    voltage = adc_get_voltage_blocking(AD_CH_PMU_VTEMP);
    adc_queue[2].v.voltage = voltage;
    log_info("dtemp = %d mv\n", voltage);

#if AD_CH_IO_VBAT_PORT
    voltage = adc_get_voltage_blocking(adc_io2ch(AD_CH_IO_VBAT_PORT));
    adc_queue[3].v.voltage = voltage;
    log_info("io_vbat = %d mv\n", voltage);
#endif

    request_irq(IRQ_GPADC_IDX, IRQ_ADC_IP, adc_isr, 0);//注册中断函数
    /* usr_timer_add(NULL, adc_scan,  5, 0); */
    /* hw_timer_init(JL_TIMER0, 5); //定时器初始化，5ms */
    /* request_irq(IRQ_TIME0_IDX, 1, adc_scan, 0);    //设置定时器中断(中断号，优先级，回调函数，cpu核) */

    /* adc_add_sample_ch(adc_io2ch(IO_PORTA_08)); */
    /* adc_set_sample_period(adc_io2ch(IO_PORTA_08), PMU_CH_SAMPLE_PERIOD); */
    /* void adc_test_demo(); */
    /* usr_timer_add(NULL, adc_test_demo, 1000, 0); //打印信息 */
}

static void adc_test_demo()  //adc测试函数，根据需求搭建
{
    /* printf("\n\n%s() CHIP_ID :%x\n", __func__, get_chip_version()); */
    /* printf("%s() VBG:%d\n", __func__, adc_get_value(AD_CH_LDOREF)); */
    printf("%s() VBAT:%d mv\n", __func__, adc_get_voltage(AD_CH_PMU_VBAT) * 4);
    printf("%s() PA3:%d\n", __func__, adc_get_value(adc_io2ch(IO_PORTA_03)));
    printf("%s() PA3:%d mv\n", __func__, adc_get_voltage(adc_io2ch(IO_PORTA_03)));
    printf("%s() PA4:%d\n", __func__, adc_get_value_blocking(adc_io2ch(IO_PORTA_04)));
    printf("%s() PA4:%d mv\n", __func__, adc_get_voltage_blocking(adc_io2ch(IO_PORTA_04)));
    /* printf("%s() DTEMP:%d\n", __func__, adc_get_voltage(AD_CH_PMU_VTEMP)); */
    /* printf("%s() PA2:%d\n", __func__, adc_get_value(AD_CH_IO_PA2)); */
    /* printf("%s() PA2:%dmv\n", __func__, adc_get_voltage_blocking(AD_CH_IO_PA2)); */
    /* printf("%s() PA10:%dmv\n", __func__, adc_get_voltage_blocking(AD_CH_IO_PA10)); */
    /* printf("%s() PA6:%dmv\n", __func__, adc_get_voltage_blocking(AD_CH_IO_PA6)); */
    /* printf("%s() PB1:%d\n", __func__, adc_get_value(AD_CH_IO_PB1)); */
    /* printf("%s() PB1:%dmv\n", __func__, adc_get_voltage(AD_CH_IO_PB1)); */
}
float adc_value_update(enum AD_CH ch, float adc_value_old, float adc_value_new) //vbat, vbg 值更新
{
    float ret_value;
#if AD_CH_IO_VBAT_PORT
    if (ch == AD_CH_IO_VBAT_PORT) {
        return ret_value = (adc_value_old * 0.8f + adc_value_new * 0.2f); //这里可以调用自己的滤波算法
    }
#endif
    return ret_value = (adc_value_old * 0.8f + adc_value_new * 0.2f) + 0.5;
}

void adc_init(void) //adc初始化
{
    JL_ADC->ADC_CON = 0x2;
    JL_ADC->ANA_CON = 0;
    JL_ADC->DMA_CON0 = 0;
    JL_ADC->DMA_CON1 = 0;
    JL_ADC->DMA_CON2 = 0;
    JL_ADC->DMA_BADR = 0;

    SFR(JL_ADC->ANA_CON, 19, 2, 3);
    SFR(JL_ADC->ANA_CON, 17, 2, 2);
    SFR(JL_ADC->ANA_CON, 15, 2, 2);
    SFR(JL_ADC->ANA_CON, 14, 1, 1);

    _adc_pmu_vbg_enable();

    JL_ADC->ADC_CON = BIT(31);
    JL_ADC->ADC_CON |= BIT(2);

    adc_close();

    adc_hw_init();
    /* adc_internal_signal_to_io(AD_CH_LDOREF, IO_PORTA_01); */
}

u32 adc_get_next_ch()    //获取采集队列中下一个通道的队列编号
{
    if (cur_ch == ADC_MAX_CH) {
        cur_ch = 0;
    }
    for (int i = cur_ch; i < ADC_MAX_CH; i++) {
        if (adc_queue[i].ch != -1) {
            if (adc_queue[i].sample_period) {
                if (time_before(adc_queue[i].jiffies, jiffies)) {
                    adc_queue[i].jiffies += adc_queue[i].sample_period;
                    /* printf("prd---> %d %d\n", jiffies, adc_queue[i].jiffies); */
                } else {
                    continue;
                }
            }

            return i;
        }
    }
    return ADC_MAX_CH;
}

u32 adc_set_sample_period(enum AD_CH ch, u32 ms) //设置一个指定通道的采样周期
{
    u32 i;
    for (i = 0; i < ADC_MAX_CH; i++) {
        if (adc_queue[i].ch == ch) {
            /* adc_queue[i].sample_period = msecs_to_jiffies(ms); */
            /* adc_queue[i].jiffies = msecs_to_jiffies(ms) + jiffies; */
            adc_queue[i].sample_period = ms / 10;
            adc_queue[i].jiffies = 0 + jiffies;
            break;
        }
    }
    return i;
}

void adc_update_vbg_value_restart(u8 cur_miovdd_level, u8 new_miovdd_level)
{
    printf("cur = %d, new = %d\n", cur_miovdd_level, new_miovdd_level);
    u32 cur_miovdd_vol = 2100 + cur_miovdd_level * 100;
    u32 new_miovdd_vol = 2100 + new_miovdd_level * 100;
    adc_queue[0].v.value = adc_queue[0].v.value * cur_miovdd_vol / new_miovdd_vol;
}

u32 adc_get_vbg_voltage()
{
    if (ADC_VBG_DATA_WIDTH == 0) {
        return ADC_VBG_CENTER;
    }
    int data = efuse_get_gpadc_vbg_trim();
    int sign = (data >> ADC_VBG_DATA_WIDTH) & 0x01;
    data = data & ((1 << ADC_VBG_DATA_WIDTH) - 1);
    if (sign == 1) {
        return  ADC_VBG_CENTER - data * ADC_VBG_TRIM_STEP;
    } else {
        return  ADC_VBG_CENTER + data * ADC_VBG_TRIM_STEP;
    }
}
//adc_value-->voltage   用传入的 vbg 计算
u32 adc_value_to_voltage(u32 adc_vbg, u32 adc_value)
{
    u32 tmp = adc_get_vbg_voltage();
    if (adc_vbg == 0) {
        adc_vbg = 1;        //防止div0异常
    }
    u32 adc_res = adc_value * tmp / adc_vbg;
    return adc_res;
}

//adc_value-->voltage   用 vbg_value_array 数组均值计算
u32 adc_value_to_voltage_filter(u32 adc_value)
{
    u32 adc_vbg = adc_get_value(AD_CH_LDOREF);
    return adc_value_to_voltage(adc_vbg, adc_value);
}

u32 adc_get_value(enum AD_CH ch)   //获取一个指定通道的原始值，从队列中获取
{
    for (int i = 0; i < ADC_MAX_CH; i++) {
        if (adc_queue[i].ch == ch) {
            /* printf("get ch %d, %d\n", adc_queue[i].v.value, i); */
            return adc_queue[i].v.value;
        }
    }
    /* printf("func:%s,line:%d\n", __func__, __LINE__); */
    return 1;
}

u32 adc_get_voltage(enum AD_CH ch) //获取一个指定通道的电压值，从队列中获取
{
    for (int i = 0; i < ADC_MAX_CH + ENABLE_PREEMPTIVE_MODE; i++) {
        if (ch == adc_queue[i].ch && adc_queue[i].adc_voltage_mode == 1) {
            return adc_queue[i].v.voltage;
        }
    }

    u32 adc_vbg = adc_get_value(AD_CH_LDOREF);
    u32 adc_res;
    if (ch == AD_CH_IOVDD) {
        adc_res = 4095;
    } else {
        adc_res = adc_get_value(ch);
    }
    return adc_value_to_voltage(adc_vbg, adc_res);
}

u32 adc_get_value_blocking(enum AD_CH ch)    //阻塞式采集一个指定通道的adc原始值
{
    adc_queue[ADC_MAX_CH].ch = ch;

    adc_wait_enter_idle();

    local_irq_disable();

    adc_sample(adc_queue[ADC_MAX_CH].ch, 0);

    adc_queue[ADC_MAX_CH].v.value = adc_wait_pnd();

    adc_queue[ADC_MAX_CH].ch = -1;

    adc_set_enter_idle();

    local_irq_enable();

    return adc_queue[ADC_MAX_CH].v.value;
}

u32 adc_get_value_blocking_filter(enum AD_CH ch, u32 sample_times)
{
    u32 ch_adc_value = 0;
    u32 ch_min_value = 0xffff;
    u32 ch_max_value = 0;

    if (sample_times <= 2) {
        sample_times = 3;
    }
    for (int i = 0; i < sample_times; i++) {
        if (ch == AD_CH_IOVDD) {
            break;
        }

        u32 adc_value = adc_get_value_blocking(ch);

        if (adc_value > ch_max_value) {
            ch_max_value = adc_value;
        }
        if (adc_value < ch_min_value) {
            ch_min_value = adc_value;
        }
        ch_adc_value += adc_value;
    }

    if (ch == AD_CH_IOVDD) {
        ch_adc_value = 4095;
    } else {
        ch_adc_value -= ch_max_value;
        ch_adc_value -= ch_min_value;

        ch_adc_value /= sample_times - 2;
    }

    /* printf("%s() %d ch: %d min: %d max: %d  ", __func__, __LINE__, ch_adc_value, ch_min_value, ch_max_value); */
    return ch_adc_value;
}

u32 adc_get_voltage_blocking(enum AD_CH ch)  //阻塞式采集一个指定通道的电压值（经过均值滤波处理）
{
    u32 vbg_adc_value = 0;
    u32 vbg_min_value = 0xffff;
    u32 vbg_max_value = 0;

    u32 ch_adc_value = 0;
    u32 ch_min_value = 0xffff;
    u32 ch_max_value = 0;

    for (int i = 0; i < 12; i++) {

        u32 adc_value = adc_get_value_blocking(AD_CH_LDOREF);
        if (adc_value > vbg_max_value) {
            vbg_max_value = adc_value;
        }
        if (adc_value < vbg_min_value) {
            vbg_min_value = adc_value;
        }

        vbg_adc_value += adc_value;

        if (ch == AD_CH_IOVDD) {
            continue;
        }

        adc_value = adc_get_value_blocking(ch);

        if (adc_value > ch_max_value) {
            ch_max_value = adc_value;
        }
        if (adc_value < ch_min_value) {
            ch_min_value = adc_value;
        }
        ch_adc_value += adc_value;
    }

    vbg_adc_value -= vbg_max_value;
    vbg_adc_value -= vbg_min_value;

    vbg_adc_value /= 10;
    /* printf("%s() %d vbg: %d min: %d max: %d  ", __func__, __LINE__, vbg_adc_value, vbg_min_value, vbg_max_value); */


    if (ch == AD_CH_IOVDD) {
        ch_adc_value = 4095;
    } else {
        ch_adc_value -= ch_max_value;
        ch_adc_value -= ch_min_value;

        ch_adc_value /= 10;
    }

    /* printf("%s() %d ch: %d min: %d max: %d  ", __func__, __LINE__, ch_adc_value, ch_min_value, ch_max_value); */
    return adc_value_to_voltage(vbg_adc_value, ch_adc_value);
}

static u32 abs_value(u32 a, u32 b)
{
    if (a >= b) {
        return a - b;
    } else {
        return b - a;
    }
}
u32 adc_cpu_mode_process(u32 adc_value)    //adc_isr 中断中，cpu模式的公共处理函数
{
    if (adc_queue[cur_ch].adc_voltage_mode == 1) {
        u32 vbg_value = adc_queue[0].v.value;
        u32 voltage = adc_value_to_voltage(vbg_value, adc_value);
        adc_queue[cur_ch].v.voltage = adc_value_update(adc_queue[cur_ch].ch, adc_queue[cur_ch].v.voltage, voltage);
        /* printf("%d ad[%x]: %d %d\n", cur_ch, adc_queue[cur_ch].ch, adc_value, adc_queue[cur_ch].v.voltage); */
    } else  if (adc_queue[cur_ch].ch == AD_CH_LDOREF) {
        adc_queue[0].v.value = adc_value_update(AD_CH_LDOREF, adc_queue[0].v.value, adc_value);
    } else {
        adc_queue[cur_ch].v.value = adc_value;
    }
    /* printf("%d ad[%x]: %d %d", cur_ch, adc_queue[cur_ch].ch, adc_value, adc_queue[cur_ch].v.voltage); */
    if (adc_queue[ADC_MAX_CH].ch != -1) {
        adc_close();
        return 1;
    }
    return 0;
}

u32 adc_check_vbat_lowpower()
{
    return 0;
}
void adc_hw_uninit(void)
{
    local_irq_disable();
    memset(adc_queue, 0xff, sizeof(adc_queue));
    JL_ADC->ADC_CON = 0;
    JL_ADC->ANA_CON = 0;
    JL_ADC->DMA_CON0 = 0;
    JL_ADC->DMA_CON1 = 0;
    JL_ADC->DMA_CON2 = 0;
    JL_ADC->DMA_BADR = 0;
    local_irq_enable();
}

void adc_hw_enter_sleep(enum LOW_POWER_LEVEL lp_mode)
{
#define VBG_TEST_EN(en)				p33_fast_access(P3_PMU_ADC0, BIT(5), en)
#define VBG_BUFFER_EN(en)			p33_fast_access(P3_PMU_ADC0, BIT(4), en)
#define PMU_TOADC_EN(en)			p33_fast_access(P3_PMU_ADC1, BIT(1), en)
//PMU_TOADC_OE
#define PMU_DET_OE(en)				p33_fast_access(P3_PMU_ADC1, BIT(0), en)

#if ADC_SENSE_ENABLE
    SFR(JL_ADC->ANA_CON, 8, 1, 0);
    SFR(JL_ADC->ANA_CON, 13, 1, 0);

#endif

    JL_ADC->ADC_CON = 0;

    PMU_TOADC_EN(0);
    PMU_DET_OE(0);
    VBG_BUFFER_EN(0);
    VBG_TEST_EN(0);
}

void adc_hw_exit_sleep(enum LOW_POWER_LEVEL lp_mode)
{
#if ADC_SENSE_ENABLE
    SFR(JL_ADC->ANA_CON, 8, 1, 1);
    SFR(JL_ADC->ANA_CON, 13, 1, 1);

#endif
}

void adc_exit_power_down_update()
{
    _adc_pmu_vbg_enable();

    /* JL_PORTA->DIR &= ~BIT(0); */
    /* JL_PORTA->OUT |= BIT(0); */

    u32 vbg_value;
    u32 adc_value;
    u32 voltage;

    vbg_value = adc_get_value_blocking_filter(AD_CH_LDOREF, 3);
    /* printf("-%d", vbg_value); */
    if (abs_value(vbg_value, adc_queue[0].v.value) >= 5) {
        goto exit;
    }
    adc_queue[0].v.value = adc_value_update(AD_CH_LDOREF, adc_queue[0].v.value, vbg_value);
    vbg_value = adc_queue[0].v.value;

    adc_value = adc_get_value_blocking_filter(AD_CH_PMU_VBAT, 3);
    voltage = adc_value_to_voltage(vbg_value, adc_value);
    if (abs_value(voltage, adc_queue[1].v.voltage) <= 10) {
        adc_queue[1].v.voltage = adc_value_update(AD_CH_PMU_VBAT, adc_queue[1].v.voltage, voltage);
    }

    adc_value = adc_get_value_blocking_filter(AD_CH_PMU_VTEMP, 3);
    voltage = adc_value_to_voltage(vbg_value, adc_value);
    if (abs_value(voltage, adc_queue[2].v.voltage) <= 10) {
        adc_queue[2].v.voltage = adc_value_update(AD_CH_PMU_VTEMP, adc_queue[2].v.voltage, voltage);
    }


exit:
    adc_queue[0].jiffies += adc_queue[0].sample_period;
    adc_queue[1].jiffies += adc_queue[1].sample_period;
    adc_queue[2].jiffies += adc_queue[2].sample_period;

    /* JL_PORTA->OUT &= ~BIT(0); */
    /* printf("vbg;%d, vbat:%dmv, vtemp:%d\n", adc_queue[0].v.value, (u32)adc_queue[1].v.voltage*4, (u32)adc_queue[2].v.voltage); */
}
