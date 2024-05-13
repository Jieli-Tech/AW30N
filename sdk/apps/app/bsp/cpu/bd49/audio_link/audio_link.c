#include "audio_link.h"
#include "iis.h"
#include "gpio.h"
#include "gpio_hw.h"
#include "audio.h"
#include "audio_link_api.h"
#include "app_config.h"
#include "app_modules.h"

#if 1
#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[normal]"
#include "log.h"


static u32 *ALNK_BUF_ADR[] = {
    (u32 *)(&(JL_ALNK0->ADR0)),
    (u32 *)(&(JL_ALNK0->ADR1)),
    (u32 *)(&(JL_ALNK0->ADR2)),
    (u32 *)(&(JL_ALNK0->ADR3)),
};

/* mclk源选择 */
enum {
    MCLK_EXTERNAL	= 0u,
    MCLK_SYS_CLK		,
    MCLK_OSC_CLK 		,
    MCLK_PLL_CLK		,
};
enum {
    MCLK_DIV_1		= 0u,
    MCLK_DIV_2			,
    MCLK_DIV_4			,
    MCLK_DIV_8			,
    MCLK_DIV_16			,
};

enum {
    MCLK_LRDIV_EX	= 0u,
    MCLK_LRDIV_64FS		,
    MCLK_LRDIV_128FS	,
    MCLK_LRDIV_192FS	,
    MCLK_LRDIV_256FS	,
    MCLK_LRDIV_384FS	,
    MCLK_LRDIV_512FS	,
    MCLK_LRDIV_768FS	,
};

/* Audio_link io */
enum {
    IIS_IO_MCLK = 0u,
    IIS_IO_SCLK 	,
    IIS_IO_LRCK 	,
    IIS_IO_CH0 		,
    IIS_IO_CH1 		,
    IIS_IO_CH2 		,
    IIS_IO_CH3 		,
};


enum alnk_clock {
    ALINK_CLOCK_12M288K,
    ALINK_CLOCK_11M2896K,
};

u16 const audio_link_sr_mclk_div[9][4] = {
    { ALINK_SR_8000,  ALINK_CLOCK_12M288K,  MCLK_DIV_4, MCLK_LRDIV_384FS},
    { ALINK_SR_11025, ALINK_CLOCK_11M2896K, MCLK_DIV_4, MCLK_LRDIV_256FS},
    { ALINK_SR_12000, ALINK_CLOCK_12M288K,  MCLK_DIV_4, MCLK_LRDIV_256FS},
    { ALINK_SR_16000, ALINK_CLOCK_12M288K,  MCLK_DIV_2, MCLK_LRDIV_384FS},
    { ALINK_SR_22050, ALINK_CLOCK_11M2896K, MCLK_DIV_2, MCLK_LRDIV_256FS},
    { ALINK_SR_24000, ALINK_CLOCK_12M288K,  MCLK_DIV_2, MCLK_LRDIV_256FS},
    { ALINK_SR_32000, ALINK_CLOCK_12M288K,  MCLK_DIV_1, MCLK_LRDIV_384FS},
    { ALINK_SR_44100, ALINK_CLOCK_11M2896K, MCLK_DIV_1, MCLK_LRDIV_256FS},
    { ALINK_SR_48000, ALINK_CLOCK_12M288K,  MCLK_DIV_1, MCLK_LRDIV_256FS},
};


ALINK_PARM *p_alink_parm;

//==================================================
static void audio_link_clock_sel(enum alnk_clock clk)
{
    switch (clk) {
    case ALINK_CLOCK_12M288K:
        SFR(JL_CLOCK->PRP_CON1, 4, 4, 3); //320/26 = 12.307
        SFR(JL_CLOCK->PRP_CON1, 8, 6, 25); //320/(25 + 1)
        break;
    case ALINK_CLOCK_11M2896K:
        SFR(JL_CLOCK->PRP_CON1, 4, 4, 4); //192/17 = 11.294
        SFR(JL_CLOCK->PRP_CON1, 8, 6, 16); //192/(16 + 1)
        break;
    default:
        break;
    }
}

u16 alink_sr(u32 rate)
{
    if (false == audio_clk_open_check()) {
        return 0;
    }
    u16 i = 0;
    u16 index = 6;	//SR初始为32k
    u32 tmp_rate = rate >> 3;
    for (i = 0; i < (sizeof(audio_link_sr_mclk_div) / sizeof(audio_link_sr_mclk_div[0])); i++) {
        if (tmp_rate == (audio_link_sr_mclk_div[i][0] >> 3)) {
            index = i;
            break;
        }
    }
    u32 ALNK_CLOCK 	= audio_link_sr_mclk_div[index][1];
    u32 MCLK_DIV 	= audio_link_sr_mclk_div[index][2];
    u32 MCLK_LRDIV 	= audio_link_sr_mclk_div[index][3];

    audio_link_clock_sel(ALNK_CLOCK);
    ALINK_MDIV(MCLK_DIV);
    ALINK_LRDIV(MCLK_LRDIV);
    /* 从机模式 */
    if (p_alink_parm->role == ALINK_ROLE_SLAVE) {
        ALINK_LRDIV(MCLK_LRDIV_EX);
    }

    return audio_link_sr_mclk_div[index][0];
}

static void *alink_addr(u8 ch)
{
    u8 *buf_addr = NULL; //can be used
    u32 buf_index = 0;
    buf_addr = (u8 *)(p_alink_parm->ch_cfg[ch].buf);

    u8 index_table[4] = {12, 13, 14, 15};
    u8 bit_index = index_table[ch];		//which channel
    buf_index = (JL_ALNK0->CON0 & BIT(bit_index)) ? 0 : 1;	//buf0 or buf1 using
    buf_addr = buf_addr + ((p_alink_parm->dma_len / 2) * buf_index);

    return buf_addr;
}


___interrupt
static void alink_isr(void)
{
    u16 reg;
    u8 *buf_addr = NULL;
    u8 ch = 0;

    reg = JL_ALNK0->CON2;

    for (u8 i = 0; i < 4; i++) {
        if (reg & BIT(i + 4)) {
            ch = i;
            ALINK_CLR_CHx_PND(i);
            buf_addr = alink_addr(ALINK_CH0 + ch);
            if (p_alink_parm->ch_cfg[ch].enable) {
                if (p_alink_parm->ch_cfg[ch].isr_cb) {
                    p_alink_parm->ch_cfg[ch].isr_cb(ch, buf_addr, p_alink_parm->dma_len / 2);
                }
            }
        }
    }

#if 0
    //channel 0
    if (reg & BIT(4)) {
        ch = 0;
        ALINK_CLR_CH0_PND();
        buf_addr = alink_addr(ALINK_CH0);
        goto __isr_cb;
    }
    //channel 1
    if (reg & BIT(5)) {
        ch = 1;
        ALINK_CLR_CH1_PND();
        buf_addr = alink_addr(ALINK_CH1);
        goto __isr_cb;
    }
    //channel 2
    if (reg & BIT(6)) {
        ch = 2;
        ALINK_CLR_CH2_PND();
        buf_addr = alink_addr(ALINK_CH2);
        goto __isr_cb;
    }
    //channel 3
    if (reg & BIT(7)) {
        ch = 3;
        ALINK_CLR_CH3_PND();
        buf_addr = alink_addr(ALINK_CH3);
        goto __isr_cb;
    }
__isr_cb:
    if (p_alink_parm->ch_cfg[ch].enable) {
        if (p_alink_parm->ch_cfg[ch].isr_cb) {
            p_alink_parm->ch_cfg[ch].isr_cb(ch, buf_addr, p_alink_parm->dma_len / 2);
        }
    }
#endif
}

u32 alink_init(ALINK_PARM *parm)
{
    if (parm == NULL) {
        return -1;
    }
    if (false == audio_clk_open_check()) {
        return -1;
    }
    ALNK_CON_RESET();

    p_alink_parm = parm;
    JL_ALNK0->CON2 = 0xF;		//写1清pend
    request_irq(IRQ_ALINK_IDX, IRQ_ALINK0_IP, alink_isr, 0);
    ALINK_MSRC(MCLK_PLL_CLK);	/*MCLK source*/

    //===================================//
    //        输出时钟配置               //
    //===================================//
    if (parm->role == ALINK_ROLE_MASTER) {
        //主机输出时钟
        if (parm->mclk_io != ALINK_CLK_OUPUT_DISABLE) {
            gpio_set_mode(IO_PORT_SPILT(parm->mclk_io), PORT_OUTPUT_HIGH);
            gpio_set_fun_output_port(parm->mclk_io, FO_ALNK0_MCLK, 1, 1);
            ALINK_MOE(1);				/*MCLK output to IO*/
        }
        if ((parm->sclk_io != ALINK_CLK_OUPUT_DISABLE) && (parm->lrclk_io != ALINK_CLK_OUPUT_DISABLE)) {
            gpio_set_mode(IO_PORT_SPILT(parm->lrclk_io), PORT_OUTPUT_HIGH);
            gpio_set_mode(IO_PORT_SPILT(parm->sclk_io), PORT_OUTPUT_HIGH);
            gpio_set_fun_output_port(parm->lrclk_io, FO_ALNK0_LRCK, 1, 1);
            gpio_set_fun_output_port(parm->sclk_io, FO_ALNK0_SCLK, 1, 1);
            ALINK_SOE(1);				/*SCLK/LRCK output to IO*/
        }
    } else {
        //从机输入时钟
        if (parm->mclk_io != ALINK_CLK_OUPUT_DISABLE) {
            gpio_set_mode(IO_PORT_SPILT(parm->mclk_io), PORT_INPUT_FLOATING);
            gpio_set_fun_input_port(parm->mclk_io, PFI_ALNK0_MCLK);
            ALINK_MOE(0);				/*MCLK input to IO*/
        }
        if ((parm->sclk_io != ALINK_CLK_OUPUT_DISABLE) && (parm->lrclk_io != ALINK_CLK_OUPUT_DISABLE)) {
            gpio_set_mode(IO_PORT_SPILT(parm->lrclk_io), PORT_INPUT_FLOATING);
            gpio_set_mode(IO_PORT_SPILT(parm->sclk_io), PORT_INPUT_FLOATING);
            gpio_set_fun_input_port(parm->lrclk_io, PFI_ALNK0_LRCK);
            gpio_set_fun_input_port(parm->sclk_io, PFI_ALNK0_SCLK);
            ALINK_SOE(0);				/*SCLK/LRCK input to IO*/
        }
    }
    //===================================//
    //        基本模式/扩展模式          //
    //===================================//
    ALINK_DSPE(p_alink_parm->mode >> 2);

    //===================================//
    //         数据位宽16/32bit          //
    //===================================//
    //注意: 配置了24bit, 一定要选ALINK_FRAME_64SCLK, 因为sclk32 x 2才会有24bit;
    if (p_alink_parm->bitwide == ALINK_LEN_24BIT) {
        ASSERT(p_alink_parm->sclk_per_frame == ALINK_FRAME_64SCLK);
        ALINK_24BIT_MODE();
        //一个点需要4bytes, LR = 2, 双buf = 2
        /* JL_ALNK0->LEN = p_alink_parm->dma_len / 8; //点数 */
    } else {
        ALINK_16BIT_MODE();
        //一个点需要2bytes, LR = 2, 双buf = 2
        /* JL_ALNK0->LEN = p_alink_parm->dma_len / 8; //点数 */
    }
    JL_ALNK0->LEN = ALNK_BUF_POINTS_NUM;
    //===================================//
    //             时钟边沿选择          //
    //===================================//
    if (p_alink_parm->clk_mode == ALINK_CLK_FALL_UPDATE_RAISE_SAMPLE) {
        SCLKINV(0);
    } else {
        SCLKINV(1);
    }
    //===================================//
    //            每帧数据sclk个数       //
    //===================================//
    if (p_alink_parm->sclk_per_frame == ALINK_FRAME_64SCLK) {
        F32_EN(0);
    } else {
        F32_EN(1);
    }

    //===================================//
    //            设置数据采样率       	 //
    //===================================//
    alink_sr(p_alink_parm->sample_rate);

    return 0;
}


u32 alink_uninit(ALINK_PARM *parm)
{
    ALINK_EN(ALINK_MODULE_CLOSE);
    ALNK_CON_RESET();

    gpio_set_mode(IO_PORT_SPILT(parm->mclk_io), PORT_HIGHZ);
    gpio_set_mode(IO_PORT_SPILT(parm->sclk_io), PORT_HIGHZ);
    gpio_set_mode(IO_PORT_SPILT(parm->lrclk_io), PORT_HIGHZ);
    if (parm->mode == ALINK_ROLE_MASTER) {
        gpio_disable_fun_output_port(parm->mclk_io);
        gpio_disable_fun_output_port(parm->sclk_io);
        gpio_disable_fun_output_port(parm->lrclk_io);
    } else {
        gpio_disable_fun_input_port(PFI_ALNK0_MCLK);
        gpio_disable_fun_input_port(PFI_ALNK0_LRCK);
        gpio_disable_fun_input_port(PFI_ALNK0_SCLK);
    }

    for (u8 i = 0; i < 4; i++) {
        gpio_set_mode(IO_PORT_SPILT(parm->ch_cfg[i].data_io), PORT_HIGHZ);
    }
    memset(parm->ch_cfg, 0, sizeof(parm->ch_cfg));

    return 0;
}


u32 alink_channel_init(ALINK_CH_CFG *alink_data_parm)
{
    if (false == audio_clk_open_check()) {
        return -1;
    }
    //===================================//
    //           ALNK工作模式            //
    //===================================//
    if (p_alink_parm->mode > ALINK_MD_IIS_RALIGN) {		//扩展模式
        ALINK_CHx_MODE_SEL((p_alink_parm->mode - ALINK_MD_IIS_RALIGN), alink_data_parm->ch_idx);
    } else {	//基本模式
        ALINK_CHx_MODE_SEL(p_alink_parm->mode, alink_data_parm->ch_idx);
    }
    //===================================//
    //           ALNK CH DMA BUF         //
    //===================================//
    u32 *buf_addr;
    buf_addr = ALNK_BUF_ADR[alink_data_parm->ch_idx];
    *buf_addr = (u32)alink_data_parm->buf;
    //===================================//
    //          ALNK CH DAT IO INIT      //
    //===================================//
    if (alink_data_parm->dir == ALINK_DIR_RX) {		//接收
        /* log_info("rx ch ch_idx %d %d  \n",alink_data_parm->ch_idx,alink_data_parm->data_io); */
        gpio_set_mode(IO_PORT_SPILT(alink_data_parm->data_io), PORT_INPUT_FLOATING);		//上拉
        gpio_set_fun_input_port(alink_data_parm->data_io, (PFI_ALNK0_DAT0 + alink_data_parm->ch_idx * 4));
        ALINK_CHx_DIR_RX_MODE(alink_data_parm->ch_idx);
    } else {	//发送
        /* log_info("tx ch ch_idx %d %d  \n",alink_data_parm->ch_idx,alink_data_parm->data_io); */
        gpio_set_mode(IO_PORT_SPILT(alink_data_parm->data_io), PORT_OUTPUT_HIGH);	//输出高
        gpio_set_fun_output_port(alink_data_parm->data_io, (FO_ALNK0_DAT0 + alink_data_parm->ch_idx * 4), 1, 1);
        ALINK_CHx_DIR_TX_MODE(alink_data_parm->ch_idx);
    }

    p_alink_parm->ch_cfg[alink_data_parm->ch_idx].dir = alink_data_parm->dir;
    p_alink_parm->ch_cfg[alink_data_parm->ch_idx].isr_cb = alink_data_parm->isr_cb;		//注册回调
    p_alink_parm->ch_cfg[alink_data_parm->ch_idx].buf = alink_data_parm->buf;
    p_alink_parm->ch_cfg[alink_data_parm->ch_idx].enable = alink_data_parm->enable;
    memset(alink_data_parm->buf, 0xFF, ALINK_DMA_LEN);

    return alink_data_parm->ch_idx;
}


u32 alink_channel_uninit(ALINK_CH_CFG *alink_data_parm)
{
    if (false == audio_clk_open_check()) {
        return -1;
    }
    u8 unregister_ch_idx = alink_data_parm->ch_idx;

    ALINK_CHx_MODE_SEL(ALINK_MD_NONE, unregister_ch_idx);

    if (alink_data_parm->dir == ALINK_DIR_RX) {
        gpio_disable_fun_input_port((PFI_ALNK0_DAT0 + unregister_ch_idx * 4));
    } else {
        gpio_disable_fun_output_port(alink_data_parm->data_io);
    }
    gpio_set_mode(IO_PORT_SPILT(alink_data_parm->data_io), PORT_HIGHZ);	//高阻

    ALINK_CHx_DIR_RESET_MODE(alink_data_parm->ch_idx);

    memset(&p_alink_parm->ch_cfg[unregister_ch_idx], 0, sizeof(p_alink_parm->ch_cfg[unregister_ch_idx]));
    return unregister_ch_idx;
}


int audio_link_en(enum ALINK_MODULE_EN en)
{
    if (p_alink_parm) {
        ALINK_EN(en);
        return 0;
    }
    return -1;
}


static void alink_info_dump()
{
    log_info("JL_ALNK0->CON0 = 0x%x", JL_ALNK0->CON0);
    log_info("JL_ALNK0->CON1 = 0x%x", JL_ALNK0->CON1);
    log_info("JL_ALNK0->CON2 = 0x%x", JL_ALNK0->CON2);
    log_info("JL_ALNK0->CON3 = 0x%x", JL_ALNK0->CON3);
    log_info("JL_ALNK0->LEN  = 0x%x", JL_ALNK0->LEN);
    log_info("JL_ALNK0->ADR0 = 0x%x", JL_ALNK0->ADR0);
    log_info("JL_ALNK0->ADR1 = 0x%x", JL_ALNK0->ADR1);
    log_info("JL_ALNK0->ADR2 = 0x%x", JL_ALNK0->ADR2);
    log_info("JL_ALNK0->ADR3 = 0x%x", JL_ALNK0->ADR3);
}


#if 0	//debug_test

void cb_handle_tx(u8 ch, u8 *buf, u32 len)
{
    JL_PORTA->DIR &= ~BIT(12);
    JL_PORTA->OUT |= BIT(12);
    /* log_info("handle_tx ch %d %d\n",ch,len); */
    /* log_info_hexdump((u8*)buf ,32); */
    JL_PORTA->OUT &= ~BIT(12);
}

void cb_handle_rx(u8 ch, u8 *buf, u32 len)
{
    JL_PORTA->DIR &= ~BIT(11);
    JL_PORTA->OUT |= BIT(11);
    /* log_info("handle_rx ch %d %d\n",ch,len); */
    /* log_info_hexdump((u8*)buf ,32); */
    JL_PORTA->OUT &= ~BIT(11);
}



u32 audio_link_buf[4][1024 / 4] __attribute__((aligned(4)));

ALINK_CH_CFG alnk_ch_cfg_test1 = {
    .enable		= 1,
    .data_io	= ALINK_DATA_IO0,					//data IO配置
    .dir		= ALINK_DIR_RX, 				//通道传输数据方向: Tx, Rx
    .buf		= &audio_link_buf[0],					//dma buf地址
    .isr_cb		= cb_handle_rx,
};

ALINK_CH_CFG alnk_ch_cfg_test2 = {
    .enable		= 1,
    .data_io	= ALINK_DATA_IO1,					//data IO配置
    .dir		= ALINK_DIR_TX, 				//通道传输数据方向: Tx, Rx
    .buf		= &audio_link_buf[1],					//dma buf地址
    .isr_cb		= cb_handle_tx,
};
void audio_link_test(void)
{
    log_info("audio_link_test-----------\n");

    audio_link_init_api();
    audio_link_init_channel_api(&alnk_ch_cfg_test1);
    audio_link_init_channel_api(&alnk_ch_cfg_test2);
    u32 ret = audio_link_en_api(ALINK_MODULE_OPEN);
    /* log_info("alink en %d \n",ret); */
    alink_info_dump();

    extern void delay_10ms(u32 tick);
    delay_10ms(100);
    audio_link_uninit_channel_api(&alnk_ch_cfg_test2);
    while (1);
}

#endif

#endif
