#pragma bss_seg(".can.data.bss")
#pragma data_seg(".can.data")
#pragma const_seg(".can.text.const")
#pragma code_seg(".can.text")
#pragma str_literal_override(".can.text.const")



#include "gpio.h"
#include "clock.h"
#include "can.h"
#include "app_config.h"
#include "errno-base.h"

/* #define LOG_TAG_CONST   NORM */
#define LOG_TAG         "[CAN]"
#include "log.h"

static u8 *can_dma_rx_addr;
static u8 old_dma_hcnt;
static void (*cane_irq_cbfun)(enum cane_event, u32);
void hw_cane_change_callback(void (*cbfun)(enum cane_event, u32))
{
    cane_irq_cbfun = cbfun;
}
AT(.cane.text.cache.L1)
__attribute__((interrupt("")))
static void hw_cane_dma_rx_isr()
{
    u32 *rx_ff = (u32 *)&JL_ECAN->FF;
    u8 dma_rx_len = 0;
    u8 can_pnd = cane_r_reg_pnd(JL_ECAN);//auto clr
    if (can_pnd & CANE_PND_RX) {
        if (cane_r_reg_status(JL_ECAN)&CANE_STATUS_RX_BUF) {
            /* for(u8 i=0;i<13;i++)printf("%x ",rx_ff[i]);printf("\n"); */
            cane_release_rx_buf(JL_ECAN);//1.释放rx_buf.2.不会启动发送
        }
    }
    if (can_pnd & CANE_PND_TX) {
    }
    if (can_pnd & CANE_PND_ERROR) {
    }
    if (can_pnd & CANE_PND_OVER) {
    }
    if (can_pnd & CANE_PND_ERROR_PASSIVE) { //enhan
    }
    if (can_pnd & CANE_PND_ARBITRATION_LOSS) { //enhan
    }
    if (can_pnd & CANE_PND_BUS_ERROR) { //enhan
        hw_cane_send_stop();//发送失败可以取消自动重发
    }
    if (cane_irq_cbfun) {
        cane_irq_cbfun(can_pnd, cane_r_reg_status(JL_ECAN));
    }
    if (can_dma_pnd(JL_CANDMA)) {
        //
        dma_rx_len = can_dma_read_rxframe_num(JL_CANDMA);
        while ((cane_r_reg_status(JL_ECAN) & CANE_STATUS_RX_BUF));
        /* for(u8 i=0;i<13;i++)printf("%x ",rx_ff[i]);printf("\n"); */

        can_dma_clr_pnd(JL_CANDMA);
        if (cane_irq_cbfun) {
            cane_irq_cbfun(CAN_EVENT_DMA_DONE, dma_rx_len);
        }
    }
}

//reset mode dma cfg
int hw_cane_dma_rx_cfg(enum can_work_mode cwm, u8 *dma_addr, u8 dma_rxcnt)
{
    if (dma_addr == NULL) {
        return -1;
    }
    //reset mode
    cane_state_mode_sel(JL_ECAN, CANE_STATE_RESET);
    can_work_mode_sel(JL_ECAN, cwm);
    //baud
    //dma
    can_dma_set_addr(JL_CANDMA, (volatile u32)dma_addr);
    u8 rxcnt_max = 6;
    if (cwm == CAN_WORK_ENHANCEMENT) {
        rxcnt_max = 4;
    }
    if (dma_rxcnt == 0) {
        dma_rxcnt = 1;
    } else if (dma_rxcnt > rxcnt_max) {
        dma_rxcnt = rxcnt_max;
    }
    can_dma_set_rx_cnt(JL_CANDMA, dma_rxcnt);
    can_dma_rx_addr = dma_addr;
    old_dma_hcnt = 0;
    can_dma_ie_en(JL_CANDMA);
    /* request_irq(IRQ_CAN_IDX,irq_ip,hw_cane_dma_rx_isr,0); */
    can_dma_enable(JL_CANDMA);
    can_dma_clr_pnd(JL_CANDMA);
    /* log_info("dma init ok!con:0x%x,addr:0x%x,rxcnt:%d,hrcnt:%d",JL_CANDMA->CON,JL_CANDMA->ADR,JL_CANDMA->RXCNT,JL_CANDMA->HRCNT);//(&) */
    //work mode
    /* cane_state_mode_sel(JL_ECAN, CANE_STATE_WORKING); */
    return 0;
}
static u16 tx_io_cache = 0xffff;
static u16 rx_io_cache = 0xffff;
static void hw_cane_gpio_init(u16 tx_io, u16 rx_io)
{
    gpio_set_mode(IO_PORT_SPILT(tx_io), PORT_OUTPUT_HIGH);
    gpio_set_function(IO_PORT_SPILT(tx_io), PORT_FUNC_CAN0_TX);

    gpio_set_mode(IO_PORT_SPILT(rx_io), PORT_INPUT_PULLUP_10K);
    gpio_set_function(IO_PORT_SPILT(rx_io), PORT_FUNC_CAN0_RX);
    tx_io_cache = tx_io;
    rx_io_cache = rx_io;
}
static void hw_cane_gpio_deinit(u16 tx_io, u16 rx_io)
{
    gpio_set_mode(IO_PORT_SPILT(tx_io), PORT_HIGHZ);
    gpio_disable_function(IO_PORT_SPILT(tx_io), PORT_FUNC_CAN0_TX);

    gpio_set_mode(IO_PORT_SPILT(rx_io), PORT_HIGHZ);
    gpio_disable_function(IO_PORT_SPILT(rx_io), PORT_FUNC_CAN0_RX);
    tx_io_cache = 0xffff;
    rx_io_cache = 0xffff;
}
//filter id(reset) 32bit只支持扩展帧
void hw_cane_filter_rx_cfg(enum cane_rx_filter_mode filter_mode, u32 id_e, u32 idm_e, u32 id_se, u32 idm_se)
{
    //reset mode
    cane_state_mode_sel(JL_ECAN, CANE_STATE_RESET);
    u32 *acr0 = (u32 *)&JL_ECAN->FF;
    u32 *amr0 = (u32 *)&JL_ECAN->BUF3;
    cane_filter_mode_sel(JL_ECAN, filter_mode);//enhan
    /* log_info_hexdump((u8*)(&JL_ECAN->FF),8*4); */
    if (filter_mode == CANE_RX_FILTER_32BIT) {
#if 0
        //标准id 不支持
        *acr0++ = (id_e >> 3) & 0x000000ff;
        *acr0++ = (id_e & 0x00000007) << 5;
        *acr0++ = 0;
        *acr0++ = 0;
        *amr0++ = (idm_e >> 3) & 0x000000ff;
        *amr0++ = (idm_e & 0x00000007) << 5;
        *amr0++ = 0;
        *amr0++ = 0;
#else
        //扩展id
        *acr0++ = (id_e >> 21) & 0x000000ff;
        *acr0++ = (id_e >> 13) & 0x000000ff;
        *acr0++ = (id_e >> 5) & 0x000000ff;
        *acr0++ = (id_e & 0x0000001f) << 3;
        *amr0++ = (idm_e >> 21) & 0x000000ff;
        *amr0++ = (idm_e >> 13) & 0x000000ff;
        *amr0++ = (idm_e >> 5) & 0x000000ff;
        *amr0++ = (idm_e & 0x0000001f) << 3;
#endif
    } else {
        *acr0++ = (id_e >> 21) & 0x000000ff; //扩展id
        *acr0++ = (id_e >> 13) & 0x000000ff;
        *amr0++ = (idm_e >> 21) & 0x000000ff;
        *amr0++ = (idm_e >> 13) & 0x000000ff;
        *acr0++ = (id_se >> 21) & 0x000000ff; //扩展id & 标准id
        *acr0++ = (id_se >> 13) & 0x000000ff;
        *amr0++ = (idm_se >> 21) & 0x000000ff;
        *amr0++ = (idm_se >> 13) & 0x000000ff;
    }
    /* log_info_hexdump((u8*)(&JL_ECAN->FF),8*4); */
}
int hw_cane_baud_cfg(u32 freq)
{
    //baud(reset)
    //模块频率错误,只有*2才能得到目标频率
    u32 can_clk = freq * 2; //500k
    u32 lsb_clk = clk_get("lsb");
    u32 temp = lsb_clk / can_clk;
    u16 baud = 0;
    u8 tseg1 = 0;
    u8 tseg1_max = 0;
    u8 tseg2 = 0;
    u8 i = 0;
    for (i = 25; i > 2; i--) {
        if (temp % i == 0) {
            baud = temp / i - 1;
            if (baud <= 0x1ff) {
                i--;
                tseg1_max = i < 16 ? i : 16;
                for (tseg1 = 0; tseg1 < tseg1_max; tseg1++) {
                    tseg2 = i - tseg1;
                    if (tseg2 < 8) {
                        break;
                    }
                }
                if (tseg1 == tseg1_max) {
                    /* log_debug("can tseg1(%d) error!", tseg1); */
                    i++;
                } else {
                    break;
                }
            }
        }
    }
    if (i == 2) {
        log_error("can clk error!");
        return -1;
    }
    //reset mode
    cane_state_mode_sel(JL_ECAN, CANE_STATE_RESET);
    tseg1--;
    tseg2--;
    cane_set_boad_h(JL_ECAN, baud >> 6);
    cane_set_boad_l(JL_ECAN, baud & 0x3f);
    cane_set_tseg1(JL_ECAN, tseg1);
    cane_set_tseg2(JL_ECAN, tseg2);
    cane_set_siw(JL_ECAN, 1);
    cane_set_sample_1(JL_ECAN);
    /* cane_set_sample_3(JL_ECAN); */

    log_info("clk ok! lsb:%dhz,can:%dhz,baud:%d,tseg1:%d,tseg2:%d", lsb_clk, freq, baud, tseg1, tseg2);
    return 0;//ok
}
void hw_cane_start()//中途修改配置需调用使能can
{
    //work mode
    cane_state_mode_sel(JL_ECAN, CANE_STATE_WORKING);
}
//clk: 0~1M
int hw_cane_init(struct cane_config *cane_cfg)
{
    int ret;
    hw_cane_gpio_init(cane_cfg->tx_io, cane_cfg->rx_io);
    //reset mode
    cane_state_mode_sel(JL_ECAN, CANE_STATE_RESET);
    can_work_mode_sel(JL_ECAN, CAN_WORK_ENHANCEMENT);
    cane_func_mode_sel(JL_ECAN, cane_cfg->func_mode);//enhan
    cane_set_ewlr(JL_ECAN, cane_cfg->ewlr);//enhan//错误报警限制默认96
    /* cane_w_reg_rxerr(JL_ECAN,8);//enhan */
    /* cane_w_reg_txerr(JL_ECAN,8);//enhan */
    //ie(&)
    if (cane_cfg->ie_en) {
        cane_set_rx_ie(JL_ECAN);
        cane_set_tx_ie(JL_ECAN);
        cane_set_error_ie(JL_ECAN);
        cane_set_over_ie(JL_ECAN);
        cane_set_error_passive_ie(JL_ECAN);//enhan
        cane_set_arbitration_loss_ie(JL_ECAN);//enhan
        cane_set_bus_error_ie(JL_ECAN);//enhan
        request_irq(IRQ_CAN_IDX, cane_cfg->irq_priority, hw_cane_dma_rx_isr, 0);
        cane_irq_cbfun = cane_cfg->irq_callback;
    }
    //rx id(reset)
    /* hw_cane_filter_rx_cfg(CANE_RX_FILTER_32BIT, 0x0000f0f0,0x00000000,0,0);//enhan */
    /* hw_cane_filter_rx_cfg(CANE_RX_FILTER_32BIT, 0x00f0f0f0,0xffffffff,0,0);//enhan */
    /* hw_cane_filter_rx_cfg(CANE_RX_FILTER_16BIT, 0x00f0f0f0,0xffffffff, 0,0);//enhan */
    /* hw_cane_filter_rx_cfg(CANE_RX_FILTER_16BIT, 0x00f0f0f0,0xffffffff, 0x10000000,0);//enhan//只有16bit模式id2兼容标准11位id */
    /* hw_cane_filter_rx_cfg(CANE_RX_FILTER_16BIT, 0,0, 0x03f0f0f0,0xffffffff);//enhan */
    hw_cane_filter_rx_cfg(cane_cfg->filter_mode, cane_cfg->id_ext, cane_cfg->idm_ext, cane_cfg->id_std, cane_cfg->idm_std); //enhan
    //dma
    if (cane_cfg->rx_dma_buf_size < cane_cfg->rx_dma_frame_size * 16) {
        cane_cfg->rx_dma_frame_size = cane_cfg->rx_dma_buf_size / 16;
    }
    ret = hw_cane_dma_rx_cfg(CAN_WORK_ENHANCEMENT, cane_cfg->rx_dma_buf, cane_cfg->rx_dma_frame_size);
    if (ret < 0) {
        log_error("can dma error!");
        return ret;
    }

    //baud(reset)
    ret = hw_cane_baud_cfg(cane_cfg->freq);
    if (ret < 0) {
        return ret;
    }

    //work mode
    hw_cane_start();
    log_info("cane init ok!");
    /* hw_cane_dump(); */
    return ret;
}
int hw_cane_resume(hw_can_dev can)/*{{{*/
{
    if ((tx_io_cache == 0xffff) || (rx_io_cache == 0xffff)) {
        return 0;
    }
    //io
    hw_cane_gpio_init(tx_io_cache, rx_io_cache);
    hw_cane_start();
    return 0;
}
int hw_cane_suspend(hw_can_dev can)
{
    if ((tx_io_cache == 0xffff) || (rx_io_cache == 0xffff)) {
        return 0;
    }
    cane_state_mode_sel(JL_ECAN, CANE_STATE_RESET);
    //io
    hw_cane_gpio_deinit(tx_io_cache, rx_io_cache);
    return 0;
}

void hw_cane_set_ie(hw_can_dev can, can_pnd_typedef pnd, u8 en)
{
    if (en) {
        JL_ECAN->CON0 |= (pnd << 1);
    } else {
        JL_ECAN->CON0 &= ~(pnd << 1);
    }
}
u8 hw_cane_get_pnd(hw_can_dev can)//会清空所有pnd，慎用
{
    return (cane_r_reg_pnd(JL_ECAN));
}
void hw_cane_clr_pnd(hw_can_dev can, can_pnd_typedef pnd)//会清空其他pnd，慎用
{
    cane_r_reg_pnd(JL_ECAN);
    /* if(pnd==CANE_PND_OVER)cane_over_pnd_clr(JL_ECAN); */
}
void hw_cane_clr_all_pnd(hw_can_dev can)
{
    cane_r_reg_pnd(JL_ECAN);
    /* cane_over_pnd_clr(JL_ECAN); */
}/*}}}*/

int hw_cane_send_buf(enum cane_id_type ide, u32 id, enum cane_tx_rtr_mode rtr, u8 *buf, u8 len)
{
    while ((cane_r_reg_status(JL_ECAN)&CANE_STATUS_TX_BUF) == 0); //未完成
    u32 ff = 0;
    ff = ide << 7 | rtr << 6 | len;
    cane_w_reg_ff(JL_ECAN, ff);
    if (ide == CANE_ID_STANDARD) {
        cane_set_tx_sid1(JL_ECAN, (id >> 3) & 0x000000ff);
        cane_set_tx_sid2(JL_ECAN, (id & 0x00000007) << 5);
    } else {
        cane_set_tx_eid1(JL_ECAN, (id >> 21) & 0x000000ff);
        cane_set_tx_eid2(JL_ECAN, (id >> 13) & 0x000000ff);
        cane_set_tx_eid3(JL_ECAN, (id >> 5) & 0x000000ff);
        cane_set_tx_eid4(JL_ECAN, (id & 0x0000001f) << 3);
    }
    if (rtr == CANE_TX_RTR_DATA) {
        if (ide == CANE_ID_STANDARD) {
            cane_set_tx_sdata1(JL_ECAN, buf[0]);
            cane_set_tx_sdata2(JL_ECAN, buf[1]);
            cane_set_tx_sdata3(JL_ECAN, buf[2]);
            cane_set_tx_sdata4(JL_ECAN, buf[3]);
            cane_set_tx_sdata5(JL_ECAN, buf[4]);
            cane_set_tx_sdata6(JL_ECAN, buf[5]);
            cane_set_tx_sdata7(JL_ECAN, buf[6]);
            cane_set_tx_sdata8(JL_ECAN, buf[7]);
        } else {
            cane_set_tx_edata1(JL_ECAN, buf[0]);
            cane_set_tx_edata2(JL_ECAN, buf[1]);
            cane_set_tx_edata3(JL_ECAN, buf[2]);
            cane_set_tx_edata4(JL_ECAN, buf[3]);
            cane_set_tx_edata5(JL_ECAN, buf[4]);
            cane_set_tx_edata6(JL_ECAN, buf[5]);
            cane_set_tx_edata7(JL_ECAN, buf[6]);
            cane_set_tx_edata8(JL_ECAN, buf[7]);
        }
    }
    /* cane_set_tx_frame_cnt(JL_ECAN, 2);//enhan//发送 无效 */

    cane_strat_tx(JL_ECAN);
    return 0;
}
int hw_cane_send_restart()
{
    while ((cane_r_reg_status(JL_ECAN)&CANE_STATUS_TX)); //上包未完成
    cane_strat_tx(JL_ECAN);
    return 0;
}
int hw_cane_auto_txrx()//触发一次只发收
{
    cane_start_auto_txrx(JL_ECAN);
    return 0;
}
int hw_cane_send_stop()
{
    cane_stop_tx(JL_ECAN);
    return 0;
}
//中断&查询
int hw_cane_get_recv_frame_num()
{
    /* cane_get_rx_frame_cnt(JL_ECAN);//enhan */
    /* if(cane_r_reg_status(JL_ECAN)&CANE_STATUS_RX_BUF){ */
    return can_dma_read_rxframe_num(JL_CANDMA) - old_dma_hcnt;
    /* } */
}
//中断&查询
int hw_cane_recv_buf(can_rx_info_TypeDef *rx_info)
{
    if (can_dma_rx_addr == NULL) {
        return -1;
    }
    if (0 == can_dma_get_rx_cnt(JL_CANDMA)) { //rx dma
        return -2;//no data
    }
    memset((u8 *)rx_info, 0, sizeof(can_rx_info_TypeDef));
    u8 *rx_ptr = can_dma_rx_addr + old_dma_hcnt * 16;
    rx_info->ide = !!(rx_ptr[0] & BIT(7)); //0:标准id, 1:扩展id
    rx_info->rtr = !!(rx_ptr[0] & BIT(6)); //标准id/扩展id：0:数据帧, 1:远程帧
    /* rx_info->rtr = !!(rx_ptr[2]&BIT(4));//标准id：    0:数据帧, 1:远程帧 */
    rx_info->dlc = rx_ptr[0] & 0x0f;
    if (rx_info->ide == 0) {
        rx_info->std_id = rx_ptr[1] << 8 | rx_ptr[2];
        rx_info->std_id >>= 5;
        rx_info->ext_id = 0;
        if (rx_info->rtr == 0) {
            memcpy(rx_info->rx_buf, &rx_ptr[3], rx_info->dlc);
        }
    } else {
        rx_info->std_id = 0;
        rx_info->ext_id = rx_ptr[1] << 24 | rx_ptr[2] << 16 | rx_ptr[3] << 8 | rx_ptr[4];
        rx_info->ext_id >>= 3;
        if (rx_info->rtr == 0) {
            memcpy(rx_info->rx_buf, &rx_ptr[5], rx_info->dlc);
        }
    }
    rx_info->frame_index = old_dma_hcnt;
    old_dma_hcnt++;
    if (cane_r_reg_status(JL_ECAN)&CANE_STATUS_RX_BUF) {
        cane_release_rx_buf(JL_ECAN);//1.释放rx_buf.2.不会启动发送
    }
    if (old_dma_hcnt == can_dma_get_rx_cnt(JL_CANDMA)) { //restart rx dma
        can_dma_set_rx_cnt(JL_CANDMA, old_dma_hcnt); //start dma
        old_dma_hcnt = 0;
    }
    /* while((can_r_reg_status(JL_ECAN)&CANE_STATUS_RX_BUF));//没数据 */

    //log
    /* log_info("ide:%d,rtr:%d,id:%8x,%8x,len:%d",rx_info->ide,rx_info->rtr,rx_info->std_id,rx_info->ext_id,rx_info->dlc); */
    /* if(rx_info->rtr==0){ */
    /*     log_info_hexdump(rx_info->rx_buf,rx_info->dlc); */
    /* } */
    return rx_info->dlc;
}
void hw_cane_dump()
{
    log_info("dma con:0x%x,addr:0x%x,rxcnt:%d,hrcnt:%d", JL_CANDMA->CON, JL_CANDMA->ADR, JL_CANDMA->RXCNT, JL_CANDMA->HRCNT); //(&)
    log_info("reg:mode,IE,CON0,CMD0,STATUS,PND, ,ALC,ECC,EWLR,RXERR,TXERR, ,RMC");
    log_info("reg:  %2x,%2x,  %2x,  %2x,    %2x, %2x, , %2x, %2x,  %2d,   %2d,   %2d, , %2d", JL_ECAN->CLKDIV, JL_ECAN->IE, JL_ECAN->CON0, JL_ECAN->CMD0, JL_ECAN->STATUS, JL_ECAN->PND /*(&)*/, JL_ECAN->ALC, JL_ECAN->ECC, JL_ECAN->EWLR, JL_ECAN->RXERR, JL_ECAN->TXERR/*enhan(&)*/, JL_ECAN->RMC); /*enhan wrok*/
}






#if 0
u8 can_dma_rx_test[16 * 6] __attribute__((aligned(4))); //4byte对齐,16*dma_frame_size
void cane_irq_cbfun_test(enum cane_event can_event, u32 arg)
{
    enum cane_status sta;
    enum cane_arbitration_loss_status loss_sta;
    enum cane_error_type err_type;
    can_rx_info_TypeDef rx_info_test;//rx data
    if (can_event & CAN_EVENT_RX) {
        /* hw_cane_recv_buf(&rx_info_test); */
    }
    if (can_event & CAN_EVENT_TX) {
    }
    if (can_event & CAN_EVENT_ERROR) {
    }
    if (can_event & CAN_EVENT_RX_OVER) {
    }
    if (can_event & CAN_EVENT_ERROR_PASSIVE) {
    }
    if (can_event & CAN_EVENT_ARBITRATION_LOSS) {
        sta = cane_r_reg_status(JL_ECAN);
        loss_sta = cane_r_reg_alc(JL_ECAN);
        err_type = cane_r_reg_ecc(JL_ECAN);
    }
    if (can_event & CAN_EVENT_BUS_ERROR) {
        sta = cane_r_reg_status(JL_ECAN);
        loss_sta = cane_r_reg_alc(JL_ECAN);
        err_type = cane_r_reg_ecc(JL_ECAN);
    }
    if (can_event & CAN_EVENT_DMA_DONE) {
        for (u8 i = 0; i < arg; i++) { //arg:package num
            hw_cane_recv_buf(&rx_info_test);

            log_info("ide:%d,rtr:%d,id:%8x,%8x,len:%d", rx_info_test.ide, rx_info_test.rtr, rx_info_test.std_id, rx_info_test.ext_id, rx_info_test.dlc);
            if (rx_info_test.rtr == 0) {
                log_info_hexdump(rx_info_test.rx_buf, rx_info_test.dlc);
            }
        }
    }
}
void cane_test()
{
    JL_PORTA->OUT &= ~0x00c0; //test io
    JL_PORTA->DIR &= ~0x00c0;
    JL_PORTA->OUT ^= 0x00c0;
    JL_PORTA->OUT ^= 0x00c0;

    JL_PORTA->DIR |= BIT(8); //key
    JL_PORTA->DIE |= BIT(8);
    JL_PORTA->PU0 |= BIT(8);
    JL_PORTA->PD0 &= ~BIT(8);
    log_info("******************can test*******************");
    struct cane_config cane_cfg_t = {
        .tx_io = IO_PORTA_03,
        .rx_io = IO_PORTA_04,
        .freq = 500000,
        .func_mode = CANE_FUNC_NORMAL,
        .ewlr = 96,
        .ie_en = 1,
        .irq_priority = 3,
        .irq_callback = cane_irq_cbfun_test,
        .rx_dma_buf = can_dma_rx_test,
        .rx_dma_buf_size = sizeof(can_dma_rx_test),
        .rx_dma_frame_size = 1,//基础模式:<=6, 增强模式:<=4

        .filter_mode = CANE_RX_FILTER_16BIT,
        .id_std = 0x20000,
        .idm_std = 0x20000,//
        .id_ext = 0xffffffff, //0x00f0f0f0,
        .idm_ext = 0xffffffff, //any ext id
    };

    memset(can_dma_rx_test, 0, sizeof(can_dma_rx_test));
    int ret = hw_cane_init(&cane_cfg_t);
    if (ret < 0) {
        log_error("cane init error!");
    }
    mdelay(2000);
    hw_cane_send_restart();
    wdt_clear();
    u32 tx_id = 0x07123456;
    u8 tx_buf[8];
    u8 tx_data = 0x27;
    u8 tx_len = 6;
    u8 tx_type = 0;
    u8 test_cnt = 0;
    while (1) {
        if ((JL_PORTA->IN & BIT(8)) == 0) {
            mdelay(400);
            while ((JL_PORTA->IN & BIT(8)) == 0);
            mdelay(40);
            for (u8 i = 0; i < 8; i++) {
                tx_buf[i] = tx_data + i;
            }
            if (tx_type % 4 == 0) {
                log_info("*********s d***********%d,%d", tx_type, tx_len);
                /* for(u16 i=0x400;i<0x8ff;i++){ */
                hw_cane_send_buf(CANE_ID_STANDARD, tx_id, CANE_TX_RTR_DATA, tx_buf, tx_len);
                /* wdt_clear(); */
                /* } */
            } else if (tx_type % 4 == 1) {
                log_info("*********s r***********%d,%d", tx_type, tx_len);
                hw_cane_send_buf(CANE_ID_STANDARD, tx_id, CANE_TX_RTR_REQUEST, tx_buf, tx_len);
            } else if (tx_type % 4 == 2) {
                log_info("*********e d***********%d,%d", tx_type, tx_len);
                hw_cane_send_buf(CANE_ID_EXTERNED, tx_id, CANE_TX_RTR_DATA, tx_buf, tx_len);
            } else if (tx_type % 4 == 3) {
                log_info("*********e r***********%d,%d", tx_type, tx_len);
                hw_cane_send_buf(CANE_ID_EXTERNED, tx_id, CANE_TX_RTR_REQUEST, tx_buf, tx_len);
            }
            tx_len++;
            if (tx_len > 8) {
                tx_len = 7;
                tx_type++;
                tx_data += 0x10;
            }
        }
        mdelay(100);
        wdt_clear();
    }
}
#endif

