#include "lpctmu.h"
#include "gpio.h"
#include "dma.h"
#include "adc_api.h"
#include "power_interface.h"


#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[normal]"
#include "log.h"


static const struct lpctmu_platform_data *__this = NULL;

static const u8 ch_port[LPCTMU_CH_SIZE] = {
    IO_PORTA_01,
    IO_PORTA_02,
    IO_PORTA_03,
    IO_PORTB_00,
    IO_PORTB_01,
    IO_PORTB_02,
    IO_PORTB_03,
    IO_PORTB_04,
};
static u32 ch_res[LPCTMU_CH_SIZE];

struct lpctmu_res {
    u32 res: 24;
    u32 reserved: 5;
    u32 cur_ch: 3;
};

static struct lpctmu_res ch_res_dma_buf[LPCTMU_CH_SIZE * 2];
static u8 idle_buf_idx = 0;

static u8 ch_idx_buf[LPCTMU_CH_SIZE];
static u8 ch_idx_max = LPCTMU_CH_SIZE;
static u8 ch_idx = 0;


void lpctmu_set_ch_idx_buf(u8 ch_en)
{
    ch_idx = 0;
    ch_idx_max = 0;
    for (u8 ch = 0; ch < LPCTMU_CH_SIZE; ch ++) {
        if (ch_en & BIT(ch)) {
            ch_idx_buf[ch_idx_max] = ch;
            ch_idx_max ++;
        }
    }
}

void lpctmu_set_ana_hv_level(u8 level)
{
    SFR(JL_LPCTM0->ANA0, 5, 3, level);
}

u8 lpctmu_get_ana_hv_level(void)
{
    u8 level = (JL_LPCTM0->ANA0 >> 5) & 0b111;
    return level;
}

void lpctmu_vsel_trim()
{
    SFR(JL_LPCTM0->ANA1, 0, 2, 0b01);//先使能LV
    u32 lv_vol = adc_get_voltage_blocking(AD_CH_LPCTM);  //阻塞式采集一个指定通道的电压值（经过均值滤波处理）
    log_debug("lv_vol = %dmv", lv_vol);

    SFR(JL_LPCTM0->ANA1, 0, 2, 0b10);//再使能HV
    u32 delta, diff, diff_min = -1;
    u8 aim_hv_level = 7;
    for (u8 hv_level = 0; hv_level < 8; hv_level ++) {
        lpctmu_set_ana_hv_level(hv_level);
        u32 hv_vol = adc_get_voltage_blocking(AD_CH_LPCTM);  //阻塞式采集一个指定通道的电压值（经过均值滤波处理）
        log_debug("hv_level = %d, hv_vol = %dmv", hv_level, hv_vol);
        if (hv_vol > lv_vol) {
            delta = hv_vol - lv_vol;
        } else {
            delta = lv_vol - hv_vol;
        }
        if (delta > __this->aim_vol_delta) {
            diff = delta - __this->aim_vol_delta;
        } else {
            diff = __this->aim_vol_delta - delta;
        }
        if (diff_min >= diff) {
            diff_min = diff;
            aim_hv_level = hv_level;
        }
    }
    log_debug("hv_level = %d  diff %d", aim_hv_level, diff_min);
    lpctmu_set_ana_hv_level(aim_hv_level);
    SFR(JL_LPCTM0->ANA1, 0, 2, 0b00);//关闭HV/LV
}

u32 lpctmu_get_charge_clk(void)
{
    JL_GPCNT->CON = BIT(30);
    SFR(JL_GPCNT->CON, 16,  4,  4); //主时钟周期数：32 * 2^n = 512
    SFR(JL_GPCNT->CON,  8,  5, 20); //主时钟选择lpctmu
    SFR(JL_GPCNT->CON,  1,  5, 12); //次时钟选择std24m
    JL_GPCNT->CON |= BIT(30) | BIT(0);
    while (!(JL_GPCNT->CON & BIT(31)));
    u32 gpcnt_num = JL_GPCNT->NUM;
    JL_GPCNT->CON &= ~BIT(0);
    u32 touch_clk = 512 * 24000 / gpcnt_num;
    printf("gpcnt_num = %d touch_clk = %dKHz\n", gpcnt_num, touch_clk);
    return touch_clk;
}

void lpctmu_set_ana_cur_level(u8 ch, u8 cur_level)
{
    SFR(JL_LPCTM0->CHIS, ch * 4, 3, cur_level);
}

u8 lpctmu_get_ana_cur_level(u8 ch)
{
    u8 level = (JL_LPCTM0->CHIS >> (ch * 4)) & 0b111;
    return level;
}

void lpctmu_isel_trim(u8 ch)
{
    SFR(JL_LPCTM0->ANA1, 4, 4, ch + 1);//使能对应通道

    u32 diff, diff_min = -1;
    u8 aim_cur_level;
    for (u8 cur_level = 0; cur_level < 8; cur_level ++) {
        SFR(JL_LPCTM0->ANA0, 1, 3, cur_level);
        lpctmu_set_ana_cur_level(ch, cur_level);
        u32 charge_clk = lpctmu_get_charge_clk();
        if (charge_clk > __this->aim_charge_khz) {
            diff = charge_clk - __this->aim_charge_khz;
        } else {
            diff = __this->aim_charge_khz - charge_clk;
        }
        if (diff_min >= diff) {
            diff_min = diff;
            aim_cur_level = cur_level;
        }
    }
    log_debug("ch%d cur_level = %d  diff %d", ch, aim_cur_level, diff_min);
    SFR(JL_LPCTM0->ANA0, 1, 3, aim_cur_level);
    lpctmu_set_ana_cur_level(ch, aim_cur_level);
}

void lpctmu_vsel_isel_trim(u8 ch_en)
{
    SFR(JL_LPCTM0->ANA1, 3, 1, 1);//软件控制模式
    SFR(JL_LPCTM0->ANA1, 2, 1, 1);//模拟偏置使能
    SFR(JL_LPCTM0->ANA0, 0, 1, 1);//模拟模块使能
    SFR(JL_LPCTM0->ANA1, 4, 4, 0);//暂时不使能任何通道

    lpctmu_vsel_trim();

__isel_trim:
    lpctmu_isel_trim(ch_idx_buf[ch_idx]);
    ch_idx ++;
    if (ch_idx < ch_idx_max) {
        goto __isel_trim;
    } else {
        ch_idx --;
    }
}

___interrupt
void lpctmu_isr(void)
{
    /* putchar('#'); */
    /* putchar('\n'); */
    SFR(JL_LPCTM0->WCON, 6, 1, 1);//clr pending
    u8 cur_ch;
    if (__this->ctl_mode) {//硬件自动轮询模式
        cur_ch = (JL_LPCTM0->RES >> 29) & 0b111;
        ch_res[cur_ch] = JL_LPCTM0->RES & 0xffffff;//24位
        /* printf("ch:%d res:%d\n", cur_ch, ch_res[cur_ch]); */
    } else {
        cur_ch = ch_idx_buf[ch_idx];
        ch_res[cur_ch] = JL_LPCTM0->RES & 0xffffff;//24位
        /* printf("ch:%d res=%d\n", cur_ch, ch_res[cur_ch]); */
        ch_idx ++;
        if (ch_idx < ch_idx_max) {
            SFR(JL_LPCTM0->ANA1, 4, 4, ch_idx_buf[ch_idx] + 1);
            SFR(JL_LPCTM0->WCON, 0, 1, 1);//kstart
        } else {
            ch_idx = 0;
            SFR(JL_LPCTM0->ANA1, 4, 4, ch_idx_buf[ch_idx] + 1);
        }
    }
}

#if 0
___interrupt
void lpctmu_udma_isr(void)
{
    /* putchar('$'); */
    /* putchar('\n'); */
    if (JL_UDMA->JL_UDMA_CH3.CON0 & BIT(28)) {//单次传输到一半
        JL_UDMA->JL_UDMA_CH3.CON0 |= BIT(25);
        idle_buf_idx = 0;
    } else if (JL_UDMA->JL_UDMA_CH3.CON0 & BIT(29)) {//单次传输完成
        JL_UDMA->JL_UDMA_CH3.CON0 |= BIT(26);
        idle_buf_idx = 1;
    }
    for (u8 ch = 0; ch < ch_idx_max; ch ++) {
        u8 start = ch + ch_idx_max * idle_buf_idx;
        ch_res[ch_res_dma_buf[start].cur_ch] = ch_res_dma_buf[start].res;
        /* printf("ch:%d res=%d\n", ch_res_dma_buf[start].cur_ch, ch_res_dma_buf[start].res); */
    }
}

#else
#define LPCTMU_DMA_SEL DMA3
void lpctmu_udma_isr_cbfunc(enum dma_index channel, enum dma_int_type int_type)
{
    if (channel != LPCTMU_DMA_SEL) {
        return;
    }
    /* putchar('$'); */
    /* putchar('0' + int_type); */
    /* putchar('\n'); */
    if (int_type == DMA_INT_TH) {
        idle_buf_idx = 0;
    } else if (int_type == DMA_INT_TC) {
        idle_buf_idx = 1;
    }
    for (u8 ch = 0; ch < ch_idx_max; ch ++) {
        u8 start = ch + ch_idx_max * idle_buf_idx;
        ch_res[ch_res_dma_buf[start].cur_ch] = ch_res_dma_buf[start].res;
        /* printf("ch:%d res=%d\n", ch_res_dma_buf[start].cur_ch, ch_res_dma_buf[start].res); */
    }
}

#endif


void lpctmu_udma_init(void)
{
#if 0
    JL_UDMA->JL_UDMA_CH3.CON0 = 0;
    JL_UDMA->JL_UDMA_CH3.CON1 = 0;
    JL_UDMA->JL_UDMA_CH3.SADR = (u32)&JL_LPCTM0->RES;       //源地址
    JL_UDMA->JL_UDMA_CH3.DADR = (u32)&ch_res_dma_buf[0];    //目标地址
    SFR(JL_UDMA->JL_UDMA_CH3.CON1,  0, 16,  ch_idx_max * 2);//单次传输的大小，采用乒乓Buf
    SFR(JL_UDMA->JL_UDMA_CH3.CON1, 16, 16,  0);             //循环传输的次数, 0: 65536
    SFR(JL_UDMA->JL_UDMA_CH3.CON0,  1,  3,  0);             //选择0号外设源
    SFR(JL_UDMA->JL_UDMA_CH3.CON0,  4,  2,  2);             //0~3,设置优先级为2
    SFR(JL_UDMA->JL_UDMA_CH3.CON0,  6,  1,  0);             //外设传输模式
    SFR(JL_UDMA->JL_UDMA_CH3.CON0,  7,  1,  1);             //dma通道循环模式
    SFR(JL_UDMA->JL_UDMA_CH3.CON0,  8,  1,  0);             //源地址指针不动
    SFR(JL_UDMA->JL_UDMA_CH3.CON0,  9,  1,  1);             //目标地址指针递增
    SFR(JL_UDMA->JL_UDMA_CH3.CON0, 10,  2,  2);             //源地址读数据宽度,32位
    SFR(JL_UDMA->JL_UDMA_CH3.CON0, 12,  2,  2);             //目的地址写数据宽度,32位
    SFR(JL_UDMA->JL_UDMA_CH3.CON0, 15,  2,  1);             //扩展模式
    SFR(JL_UDMA->JL_UDMA_CH3.CON0, 17,  1,  1);             //无限循环次数
    SFR(JL_UDMA->JL_UDMA_CH3.CON0, 22,  1,  1);             //单次传输到一半的中断
    SFR(JL_UDMA->JL_UDMA_CH3.CON0, 23,  1,  1);             //单次传输完成中断
    SFR(JL_UDMA->JL_UDMA_CH3.CON0, 24,  1,  0);             //所有循环结束中断

    JL_DMAGEN->JL_DMAGEN_CH3.CON0 = 0;
    JL_DMAGEN->JL_DMAGEN_CH3.SEL0 = 0;
    JL_DMAGEN->JL_DMAGEN_CH3.SEL1 = 0;

    JL_DMAGEN->JL_DMAGEN_CH3.SEL1 |= BIT(6);                //LPCTMU模块
    SFR(JL_DMAGEN->JL_DMAGEN_CH3.CON0,  1,  2,  0b10);      //下降沿
    SFR(JL_DMAGEN->JL_DMAGEN_CH3.CON0,  3,  5,  0);         //每次触发产生1个DMA请求
    SFR(JL_DMAGEN->JL_DMAGEN_CH3.CON0, 29,  1,  0);         //触发溢出异常

    HWI_Install(IRQ_UDMA_CH3_IDX, (u32)lpctmu_udma_isr, IRQ_CTMU_IP);

    SFR(JL_DMAGEN->JL_DMAGEN_CH3.CON0,  0,  1,  1);         //dmagen使能
    SFR(JL_UDMA->JL_UDMA_CH3.CON0,  0,  1,  1);             //udma通道使能

    log_info("JL_UDMA->JL_UDMA_CH3.CON0 = 0x%x", JL_UDMA->JL_UDMA_CH3.CON0);
    log_info("JL_UDMA->JL_UDMA_CH3.CON1 = 0x%x", JL_UDMA->JL_UDMA_CH3.CON1);
    log_info("JL_UDMA->JL_UDMA_CH3.SADR = 0x%x", JL_UDMA->JL_UDMA_CH3.SADR);
    log_info("JL_UDMA->JL_UDMA_CH3.DADR = 0x%x", JL_UDMA->JL_UDMA_CH3.DADR);
    log_info("JL_DMAGEN->JL_DMAGEN_CH3.CON0 = 0x%x", JL_DMAGEN->JL_DMAGEN_CH3.CON0);
    log_info("JL_DMAGEN->JL_DMAGEN_CH3.SEL0 = 0x%x", JL_DMAGEN->JL_DMAGEN_CH3.SEL0);
    log_info("JL_DMAGEN->JL_DMAGEN_CH3.SEL1 = 0x%x", JL_DMAGEN->JL_DMAGEN_CH3.SEL1);
#else

    u8 dma_idx = LPCTMU_DMA_SEL;
__check_start:
    if (dma_busy_check(dma_idx)) {
        if (dma_idx) {
            dma_idx --;
            goto __check_start;
        } else {
            log_error("no udma to lpctmu!");
            return;
        }
    }

    log_info("lpctmu dma idx = %d", dma_idx);

    dma_cfg lpctmu_dma;
    memset((u8 *)&lpctmu_dma, 0, sizeof(dma_cfg));
    lpctmu_dma.src_addr = (u32)&JL_LPCTM0->RES;
    lpctmu_dma.dest_addr = (u32)&ch_res_dma_buf[0];
    lpctmu_dma.buffersize = ch_idx_max * 2;
    lpctmu_dma.ext_mode = DMA_EXT_MODE_0_EXPAND;
    lpctmu_dma.src_inc = 0;
    lpctmu_dma.dest_inc = 1;
    lpctmu_dma.src_data_size = DMA_DATA_SIZE_WORD;
    lpctmu_dma.dest_data_size = DMA_DATA_SIZE_WORD;
    lpctmu_dma.mode = DMA_MODE_INFINITE_CYCLE;
    lpctmu_dma.priority = IRQ_CTMU_IP;
    lpctmu_dma.M2M = 0;
    lpctmu_dma.isr_cb = lpctmu_udma_isr_cbfunc;
    lpctmu_dma.gen_pol = DMA_GEN_POL_FALLING_EDGE;
    lpctmu_dma.gen_req_num = 0;
    lpctmu_dma.gen_sel0 = DMA_GEN_SEL0_DISABLE;
    lpctmu_dma.gen_sel1 = DMA_GEN_SEL1_LPCTM0_DMA_REQ;
    dma_init(dma_idx, &lpctmu_dma);
    dma_int_config(dma_idx, DMA_INT_TH, 1);
    dma_int_config(dma_idx, DMA_INT_TC, 1);
    dma_cmd(dma_idx, DMA_CMD_GEN_ENABLE);
    dma_cmd(dma_idx, DMA_CMD_ENABLE);

#endif
}

/*
   @brief   ctmu数模块初始化
*/
void lpctmu_init(const struct lpctmu_platform_data *pdata)
{
    __this = pdata;
    if (!__this) {
        return;
    }

    ch_idx = 0;
    ch_idx_max = 0;
    for (u8 ch = 0; ch < LPCTMU_CH_SIZE; ch ++) {
        ch_res[ch] = 0;
        ch_idx_buf[ch] = 0;
        if (__this->ch_en.val & BIT(ch)) {
            ch_idx_buf[ch_idx_max] = ch;
            ch_idx_max ++;
            gpio_set_mode(IO_PORT_SPILT(ch_port[ch]), PORT_HIGHZ);
        }
    }
    if (ch_idx_max == 0) {
        return;
    }

    //时钟源配置
    u32 lpctmu_clk = 200000;//clk_get("lrc");
    if (__this->clk_sel == 0) {
        SFR(JL_LPCTM0->CLKC, 0, 2, 0);  //sel lrc200KHz, 最小周期 t = 5us
        SFR(JL_LPCTM0->CLKC, 3, 3, 0);  //0分频
    } else {
        SFR(JL_LPCTM0->CLKC, 0, 2, 1);  //sel std24MHz
        SFR(JL_LPCTM0->CLKC, 6, 1, 0);  //先0分频, (1: 2分频)
        SFR(JL_LPCTM0->CLKC, 7, 1, 0);  //再0分频, (1: 256分频)
        SFR(JL_LPCTM0->CLKC, 3, 3, 7);  //再128分频 --> 24M / 128 = 187.5KHz, 最小周期 t = 5.3us
        lpctmu_clk = 187500;
    }
    //通道采集前的待稳定时间配置
    SFR(JL_LPCTM0->PPRD, 4, 4, 9);      //prp_prd = (9 + 1) * t 约等 50us > 10us
    //多通道轮询采集, 切通道时, 通道与通道的间隙时间配置
    SFR(JL_LPCTM0->PPRD, 0, 4, 9);      //stop_prd= (9 + 1) * t 约等 50us > 10us
    //每个通道采集的周期，常设几个毫秒
    u16 det_prd = __this->sample_time_ms * lpctmu_clk / 1000 - 1;
    SFR(JL_LPCTM0->DPRD, 0, 11, det_prd);

    SFR(JL_LPCTM0->CON0, 4, 1, 1);      //模拟滤波使能
    SFR(JL_LPCTM0->CON0, 2, 1, 0);      //低功耗唤醒关闭
    SFR(JL_LPCTM0->CON0, 5, 1, 0);      //连续轮询关闭
    SFR(JL_LPCTM0->CON0, 1, 1, 0);      //模块的中断不使能
    SFR(JL_LPCTM0->CON0, 0, 1, 1);      //模块总开关

    lpctmu_vsel_isel_trim(__this->ch_en.val);

    if (__this->ctl_mode) {          //硬件自动轮询模式
        SFR(JL_LPCTM0->ANA1, 3, 1, 0);
        SFR(JL_LPCTM0->CHEN, 0, 8, __this->ch_en.val);
        if (__this->ctl_mode == HW_UDMA_POLL_MODE) {
            lpctmu_udma_init();
        } else {
            SFR(JL_LPCTM0->CON0, 1, 1, 1);  //模块的中断使能
        }
    } else {
        SFR(JL_LPCTM0->CON0, 1, 1, 1);  //模块的中断使能
    }
    HWI_Install(IRQ_CTMU0_IDX, (u32)lpctmu_isr, IRQ_CTMU_IP);

    SFR(JL_LPCTM0->WCON, 0, 1, 0);      //kstart

    log_info("JL_LPCTM0->CLKC = 0x%x", JL_LPCTM0->CLKC);
    log_info("JL_LPCTM0->PPRD = 0x%x", JL_LPCTM0->PPRD);
    log_info("JL_LPCTM0->DPRD = 0x%x", JL_LPCTM0->DPRD);
    log_info("JL_LPCTM0->CON0 = 0x%x", JL_LPCTM0->CON0);
    log_info("JL_LPCTM0->WCON = 0x%x", JL_LPCTM0->WCON);
    log_info("JL_LPCTM0->CHEN = 0x%x", JL_LPCTM0->CHEN);
    log_info("JL_LPCTM0->ECON = 0x%x", JL_LPCTM0->ECON);
    log_info("JL_LPCTM0->EXEN = 0x%x", JL_LPCTM0->EXEN);
    log_info("JL_LPCTM0->CHEN = 0x%x", JL_LPCTM0->CHEN);
    log_info("JL_LPCTM0->ANA0 = 0x%x", JL_LPCTM0->ANA0);
    log_info("JL_LPCTM0->ANA1 = 0x%x", JL_LPCTM0->ANA1);
    log_info("JL_LPCTM0->CHIS = 0x%x", JL_LPCTM0->CHIS);
}

void lpctmu_kstart(void)
{
    static u8 cnt = 0;
    cnt ++;
    if (cnt >= 2) {
        cnt = 0;
        /* putchar('t'); */
        SFR(JL_LPCTM0->WCON, 0, 1, 1);      //kstart
    }
}

u32 get_lpctmu_value(u8 ch)
{
    if (!__this) {
        return 0;
    }
    u32 res = ch_res[ch];
    return res;
}

