#ifndef _CAN_HW_H_
#define _CAN_HW_H_

#include "typedef.h"


enum {
    HW_CAN_S,
    HW_CAN_E,
    MAX_HW_CAN_NUM,
};
typedef const int hw_can_dev;

//接收dma配置:
//dma cfg(reset mode)
#define can_dma_enable(reg)                     ((reg)->CON |= BIT(0))
#define can_dma_disable(reg)                    ((reg)->CON &= ~BIT(0))
#define is_can_dma_open(reg)                    ((reg)->CON & BIT(0))
#define can_dma_ie_en(reg)                      ((reg)->CON |= BIT(5))
#define can_dma_ie_dis(reg)                     ((reg)->CON &= ~BIT(5))
#define can_dma_clr_pnd(reg)                    ((reg)->CON |= BIT(6))
#define can_dma_pnd(reg)                        ((reg)->CON & BIT(7))

#define can_dma_set_addr(reg, adr)              ((reg)->ADR = adr)
//基础模式:<=6, 增强模式:<=4
#define can_dma_set_rx_cnt(reg, num)            ((reg)->RXCNT = num)
#define can_dma_get_rx_cnt(reg)                 ((reg)->HRCNT)
#define can_dma_read_rxframe_num(reg)           ((reg)->HRCNT)
//每个帧都会写16byte ram 不足写0：id1,id0,rtr1,len,data0,,,,,byte11


//模式:(reset)
enum can_work_mode {
    CAN_WORK_STANDARD,
    CAN_WORK_ENHANCEMENT,
};
#define can_work_mode_sel(reg, mode)            SFR((reg)->CLKDIV, 7, 1, mode)//reset




//基础模式:
//con0 cmd0 status pnd(reset&work)
enum can_state_mode {
    CAN_STATE_WORKING,
    CAN_STATE_RESET,
};
#define can_state_mode_sel(reg, mode)      SFR((reg)->CON0, 0, 1, mode)
typedef enum {
    CAN_PND_RX = BIT(0),
    CAN_PND_TX = BIT(1),
    CAN_PND_ERROR = BIT(2),
    CAN_PND_OVER = BIT(3),
} can_pnd_typedef;
#define can_r_reg_pnd(reg)          ((reg)->PND)
#define can_set_rx_ie(reg)          (reg->CON0 |= BIT(1))
#define can_clr_rx_ie(reg)          (reg->CON0 &= ~BIT(1))
#define can_rx_pnd(reg)             (reg->PND & BIT(0))//auto clr
// #define can_(reg)            (reg->PND |= BIT(10))
#define can_release_rx_buf(reg)     (reg->CMD0 |= BIT(2))

#define can_set_tx_ie(reg)          (reg->CON0 |= BIT(2))
#define can_clr_tx_ie(reg)          (reg->CON0 &= ~BIT(2))
#define can_tx_pnd(reg)             (reg->PND & BIT(1))//auto clr
// #define can_(reg)            (reg->PND |= BIT(11))
#define can_strat_tx(reg)           (reg->CMD0 |= BIT(0))
#define can_stop_tx(reg)            (reg->CMD0 |= BIT(1))//中止(正在发送能否?)

#define can_set_error_ie(reg)       (reg->CON0 |= BIT(3))
#define can_clr_error_ie(reg)       (reg->CON0 &= ~BIT(3))
#define can_error_pnd(reg)          (reg->PND & BIT(2))//auto clr(err,bus)
// #define can_(reg)            (reg->PND |= BIT(13))

#define can_set_over_ie(reg)        (reg->CON0 |= BIT(4))
#define can_clr_over_ie(reg)        (reg->CON0 &= ~BIT(4))
#define can_over_pnd(reg)           (reg->PND & BIT(3))//auto clr
#define can_over_pnd_clr(reg)       (reg->CMD0 |= BIT(3))//???????????

enum can_status {
    CAN_STATUS_RX_BUF = BIT(0),//rx buf have data
    CAN_STATUS_RX_OVER = BIT(1), //rx fifo over
    CAN_STATUS_TX_BUF = BIT(2),//tx buf idle
    CAN_STATUS_TX_DONE = BIT(3), //发送请求处理完
    CAN_STATUS_RX     = BIT(4),     //正在接收
    CAN_STATUS_TX     = BIT(5),     //正在发送
    CAN_STATUS_ERROR  = BIT(6),  //溢出或超限制
    CAN_STATUS_BUS    = BIT(7),    //can no active
};
#define can_r_reg_status(reg)          ((reg)->STATUS)


//acf,amf,btr0,btr1(reset)
//buad
#define can_set_siw(reg, x)         SFR((reg)->BTIME0, 6, 2, x)
#define can_set_boad_l(reg, x)      SFR((reg)->BTIME0, 0, 6, x)
#define can_set_boad_h(reg, x)      SFR((reg)->BAUDH, 0, 3, x)
#define can_set_tseg1(reg, x)       SFR((reg)->BTIME1, 0, 4, x)
#define can_set_tseg2(reg, x)       SFR((reg)->BTIME1, 4, 3, x)
#define can_set_sample_3(reg)       ((reg)->BTIME1 |= BIT(7))
#define can_set_sample_1(reg)       ((reg)->BTIME1 &=~ BIT(7))
//rx过滤器(reset)
#define can_read_rx_addr(reg)              ((reg)->ACF)//地址高8位
#define can_set_rx_addr(reg,rx_addr)       ((reg)->ACF = rx_addr)
#define can_read_rx_m_addr(reg)            ((reg)->AMF)//地址高8位
#define can_set_rx_m_addr(reg,rx_m_addr)   ((reg)->AMF = rx_m_addr)

//tx list(work)
#define can_set_tx_idh(reg,x)        ((reg)->TIDH = x)//id 11bit
#define can_set_tx_idl(reg,x)        SFR((reg)->TIDL, 5, 3, x)
#define can_set_tx_data_len(reg,x)   SFR((reg)->TIDL, 0, 4, x)//>8:=8
enum can_tx_rtr_mode {
    CAN_TX_RTR_DATA,   //len<=8
    CAN_TX_RTR_REQUEST,//no len
};
#define can_set_tx_rtr(reg,x)         SFR((reg)->TIDL, 4, 1, x)
#define can_set_tx_data1(reg,x)       ((reg)->TXB1 = x)
#define can_set_tx_data2(reg,x)       ((reg)->TXB2 = x)
#define can_set_tx_data3(reg,x)       ((reg)->TXB3 = x)
#define can_set_tx_data4(reg,x)       ((reg)->TXB4 = x)
#define can_set_tx_data5(reg,x)       ((reg)->TXB5 = x)
#define can_set_tx_data6(reg,x)       ((reg)->TXB6 = x)
#define can_set_tx_data7(reg,x)       ((reg)->TXB7 = x)
#define can_set_tx_data8(reg,x)       ((reg)->TXB8 = x)



int hw_can_resume(hw_can_dev can);
int hw_can_suspend(hw_can_dev can);
void hw_can_set_ie(hw_can_dev can, can_pnd_typedef pnd, u8 en);
u8 hw_can_get_pnd(hw_can_dev can, can_pnd_typedef pnd);//会清空其他pnd，慎用
void hw_can_clr_pnd(hw_can_dev can, can_pnd_typedef pnd);//会清空其他pnd，慎用
void hw_can_clr_all_pnd(hw_can_dev can);













//增强模式:
//con0(w:reset,r:&)
enum cane_state_mode {
    CANE_STATE_WORKING,
    CANE_STATE_RESET,
};
#define cane_state_mode_sel(reg, mode)      SFR((reg)->CON0, 0, 1, mode)
enum cane_func_mode {
    CANE_FUNC_NORMAL,   //
    CANE_FUNC_MONITOR,   //只收 LOM
    CANE_FUNC_SELF_TEST, //只发 STM
};
#define cane_func_mode_sel(reg, mode)      SFR((reg)->CON0, 1, 2, mode)
enum cane_rx_filter_mode {//afm
    CANE_RX_FILTER_16BIT,
    CANE_RX_FILTER_32BIT,
};
#define cane_filter_mode_sel(reg, mode)      SFR((reg)->CON0, 3, 1, mode)
#define cane_get_filter_mode(reg)            (!!((reg)->CON0&BIT(3)))

// cmd0 status pnd ie alc,ecc(reset&work)
#define cane_start_auto_txrx(reg)     (reg->CMD0 |= BIT(4))//触发一次自发自收
enum cane_pnd {
    CANE_PND_RX = BIT(0),
    CANE_PND_TX = BIT(1),
    CANE_PND_ERROR = BIT(2),
    CANE_PND_OVER = BIT(3),
    CANE_PND_ERROR_PASSIVE = BIT(5),
    CANE_PND_ARBITRATION_LOSS = BIT(6),
    CANE_PND_BUS_ERROR = BIT(7),
};
#define cane_r_reg_pnd(reg)          ((reg)->PND)
#define cane_set_rx_ie(reg)          (reg->IE |= BIT(0))
#define cane_clr_rx_ie(reg)          (reg->IE &= ~BIT(0))
#define cane_rx_pnd(reg)             (reg->PND & BIT(0))//auto clr
#define cane_release_rx_buf(reg)     (reg->CMD0 |= BIT(2))

#define cane_set_tx_ie(reg)          (reg->IE |= BIT(1))
#define cane_clr_tx_ie(reg)          (reg->IE &= ~BIT(1))
#define cane_tx_pnd(reg)             (reg->PND & BIT(1))//auto clr
#define cane_strat_tx(reg)           (reg->CMD0 |= BIT(0))
#define cane_stop_tx(reg)            (reg->CMD0 |= BIT(1))//中止(正在发送能否?)

#define cane_set_error_ie(reg)       (reg->IE |= BIT(2))
#define cane_clr_error_ie(reg)       (reg->IE &= ~BIT(2))
#define cane_error_pnd(reg)          (reg->PND & BIT(2))//auto clr(err,bus)

#define cane_set_over_ie(reg)        (reg->IE |= BIT(3))
#define cane_clr_over_ie(reg)        (reg->IE &= ~BIT(3))
#define cane_over_pnd(reg)           (reg->PND & BIT(3))//auto clr
#define cane_over_pnd_clr(reg)       (reg->CMD0 |= BIT(3))//???????????

#define cane_set_error_passive_ie(reg)        (reg->IE |= BIT(5))
#define cane_clr_error_passive_ie(reg)        (reg->IE &= ~BIT(5))
#define cane_error_passive_pnd(reg)       (reg->PND & BIT(5))//auto clr

#define cane_set_arbitration_loss_ie(reg)        (reg->IE |= BIT(6))
#define cane_clr_arbitration_loss_ie(reg)        (reg->IE &= ~BIT(6))
#define cane_arbitration_loss_pnd(reg)    (reg->PND & BIT(6))//auto clr

#define cane_set_bus_error_ie(reg)        (reg->IE |= BIT(7))
#define cane_clr_bus_error_ie(reg)        (reg->IE &= ~BIT(7))
#define cane_bus_error_pnd(reg)           (reg->PND & BIT(7))//auto clr

enum cane_status {
    CANE_STATUS_RX_BUF = BIT(0),//rx buf have data
    CANE_STATUS_RX_OVER = BIT(1), //rx fifo over
    CANE_STATUS_TX_BUF = BIT(2),//tx buf idle
    CANE_STATUS_TX_DONE = BIT(3), //发送请求处理完
    CANE_STATUS_RX     = BIT(4),     //正在接收
    CANE_STATUS_TX     = BIT(5),     //正在发送
    CANE_STATUS_ERROR  = BIT(6),  //溢出或超限制
    CANE_STATUS_BUS    = BIT(7),    //can no active
};
#define cane_r_reg_status(reg)          ((reg)->STATUS)

enum cane_arbitration_loss_status {//仲裁
    CANE_AL_STATUS_IDL,//id1~11
    CANE_AL_STATUS_SRTR = 11, //extern mode
    CANE_AL_STATUS_IDE,
    CANE_AL_STATUS_IDH,//id12~29
    CANE_AL_STATUS_RTR = 31,
};
#define cane_r_reg_alc(reg)          ((reg)->ALC)
enum cane_error_type {//bit67
    CANE_ECC_ERRC_BIT_ERROR,//位错误
    CANE_ECC_ERRC_FORMAT_ERROR,//格式错误
    CANE_ECC_ERRC_FILL_ERROR,//填充错误
    CANE_ECC_ERRC_OTHER_ERROR,//错误
};
enum cane_error_dir {//bit5
    CANE_ECC_DIR_TX_ERROR,
    CANE_ECC_DIR_RX_ERROR,
};
enum cane_error_location {//bit0~4 eseg
    CANE_ECC_ESEG_FRAME_START = 0b00011,
    CANE_ECC_ESEG_SEID5       = 0b00010,//bit28~21
    CANE_ECC_ESEG_SEID4       = 0b00110,//bit20~18
    CANE_ECC_ESEG_SRTR        = 0b00100,//ext
    CANE_ECC_ESEG_RTR         = 0b01100,
    CANE_ECC_ESEG_IDE         = 0b00101,
    CANE_ECC_ESEG_EID3        = 0b00111,//bit17~13
    CANE_ECC_ESEG_EID2        = 0b01111,//bit12~5
    CANE_ECC_ESEG_EID1        = 0b01110,//bit4~0
    CANE_ECC_ESEG_DLC         = 0b01011,//数据长
    CANE_ECC_ESEG_DATA        = 0b01010,//数据区
    CANE_ECC_ESEG_CRC_SEQUENCE  = 0b01000,//
    CANE_ECC_ESEG_CRC_DELIMITER = 0b11000,//
    CANE_ECC_ESEG_ACK_SLOT      = 0b11001,//
    CANE_ECC_ESEG_ACK_DELIMITER = 0b11011,//
    CANE_ECC_ESEG_FRAME_EOF     = 0b11010,//帧结束
    CANE_ECC_ESEG_ABORT         = 0b10010,//中止
    CANE_ECC_ESEG_ACTIVE_ERROR  = 0b10001,//活动错误
    CANE_ECC_ESEG_PASSIVE_ERROR = 0b10110,//消极错误
    CANE_ECC_ESEG_CTR_BIT       = 0b10011,//控制位误差
    CANE_ECC_ESEG_DEFINE_SYMBOL = 0b10111,//
    CANE_ECC_ESEG_OVER_FLAG     = 0b11100,//
};
#define cane_r_reg_ecc(reg)          ((reg)->ECC)


//btr0,btr1(reset)
//buad
#define cane_set_siw(reg, x)         SFR((reg)->BTIME0, 6, 2, x)
#define cane_set_boad_l(reg, x)      SFR((reg)->BTIME0, 0, 6, x)
#define cane_set_boad_h(reg, x)      SFR((reg)->BAUDH, 0, 3, x)
#define cane_set_tseg1(reg, x)       SFR((reg)->BTIME1, 0, 4, x)
#define cane_set_tseg2(reg, x)       SFR((reg)->BTIME1, 4, 3, x)
#define cane_set_sample_3(reg)       ((reg)->BTIME1 |= BIT(7))
#define cane_set_sample_1(reg)       ((reg)->BTIME1 &=~ BIT(7))
//ewlr,rxerr,txerr(w:reset,r:&)
#define cane_set_ewlr(reg, x)          ((reg)->EWLR = x)//错误报警限制默认96
#define cane_get_ewlr(reg)             ((reg)->EWLR & 0x1f)//错误报警限制
#define cane_r_reg_rxerr(reg)         ((reg)->RXERR & 0xff)//接收错误计数
#define cane_w_reg_rxerr(reg,x)       ((reg)->RXERR = x)//接收错误计数
#define cane_r_reg_txerr(reg)         ((reg)->TXERR & 0xff)//发送错误计数
#define cane_w_reg_txerr(reg,x)       ((reg)->TXERR = x)//发送错误计数

//rx(reset)
// #define cane_read_rx_addr(reg)              ((reg)->ACF)//地址高8位
// #define cane_set_rx_addr(reg,rx_addr)       ((reg)->ACF = rx_addr)
// #define cane_read_rx_m_addr(reg)            ((reg)->AMF)//地址高8位
// #define cane_set_rx_m_addr(reg,rx_m_addr)   ((reg)->AMF = rx_m_addr)

//tx list(work)
enum cane_id_type {
    CANE_ID_STANDARD,   //id 11
    CANE_ID_EXTERNED,   //id 29
};
#define cane_set_frame_format(reg,x)      SFR((reg)->FF, 7, 1, x)//无法bit操作
#define cane_get_frame_format(reg)        (!!((reg)->FF & BIT(7)))
#define cane_set_frame_len(reg,x)         SFR((reg)->FF, 0, 4, x)//>8:=8
#define cane_get_frame_len(reg)           ((reg)->FF & 0x0f)//>8:=8
enum cane_tx_rtr_mode {
    CANE_TX_RTR_DATA,   //数据帧 len<=8
    CANE_TX_RTR_REQUEST,//遥控帧 len:请求长
};
#define cane_set_frame_rtr(reg,x)         SFR((reg)->FF, 6, 1, x)
#define cane_get_frame_rtr(reg)         (!!((reg)->FF & BIT(6)))
#define cane_w_reg_ff(reg,x)       ((reg)->FF= x)//接收错误计数

#define cane_set_tx_sid1(reg,x)         ((reg)->BUF0 = x)
#define cane_set_tx_sid2(reg,x)         ((reg)->BUF1 = x)
#define cane_set_tx_sdata1(reg,x)       ((reg)->BUF2 = x)
#define cane_set_tx_sdata2(reg,x)       ((reg)->BUF3 = x)
#define cane_set_tx_sdata3(reg,x)       ((reg)->BUF4 = x)
#define cane_set_tx_sdata4(reg,x)       ((reg)->BUF5 = x)
#define cane_set_tx_sdata5(reg,x)       ((reg)->BUF6 = x)
#define cane_set_tx_sdata6(reg,x)       ((reg)->BUF7 = x)
#define cane_set_tx_sdata7(reg,x)       ((reg)->BUF8 = x)
#define cane_set_tx_sdata8(reg,x)       ((reg)->BUF9 = x)

#define cane_set_tx_eid1(reg,x)         ((reg)->BUF0 = x)
#define cane_set_tx_eid2(reg,x)         ((reg)->BUF1 = x)
#define cane_set_tx_eid3(reg,x)         ((reg)->BUF2 = x)
#define cane_set_tx_eid4(reg,x)         ((reg)->BUF3 = x)
#define cane_set_tx_edata1(reg,x)       ((reg)->BUF4 = x)
#define cane_set_tx_edata2(reg,x)       ((reg)->BUF5 = x)
#define cane_set_tx_edata3(reg,x)       ((reg)->BUF6 = x)
#define cane_set_tx_edata4(reg,x)       ((reg)->BUF7 = x)
#define cane_set_tx_edata5(reg,x)       ((reg)->BUF8 = x)
#define cane_set_tx_edata6(reg,x)       ((reg)->BUF9 = x)
#define cane_set_tx_edata7(reg,x)       ((reg)->BUF10 = x)
#define cane_set_tx_edata8(reg,x)       ((reg)->BUF11 = x)


//rmc(work)
#define cane_set_tx_frame_cnt(reg,x)       SFR((reg)->RMC, 0, 5, x)
#define cane_get_rx_frame_cnt(reg)         (!!((reg)->RMC & 0x1f))

enum cane_event {
    CAN_EVENT_RX = BIT(0),
    CAN_EVENT_TX = BIT(1),
    CAN_EVENT_ERROR = BIT(2),
    CAN_EVENT_RX_OVER = BIT(3),
    CAN_EVENT_ERROR_PASSIVE = BIT(5),
    CAN_EVENT_ARBITRATION_LOSS = BIT(6),
    CAN_EVENT_BUS_ERROR = BIT(7),
    CAN_EVENT_DMA_DONE = BIT(8),
};
struct cane_config {
    u16 tx_io;//
    u16 rx_io;//
    u32 freq;
    // enum can_work_mode work_mode;
    enum cane_func_mode func_mode;
    u8 ewlr;//错误报警限制默认96

    u8 ie_en;
    u8 irq_priority;//中断优先级
    void (*irq_callback)(enum cane_event, u32);
    // u32 event_mask;

    void *rx_dma_buf;
    u32 rx_dma_buf_size;
    u32 rx_dma_frame_size;//e:1<=size<=4

    enum cane_rx_filter_mode filter_mode;//16bit:
    u32 id_std;
    u32 idm_std;
    u32 id_ext;
    u32 idm_ext;
};

typedef struct {
    u8 ide;      //0:标准id, 1:扩展id
    u8 rtr;      //标准id/扩展id：0:数据帧, 1:远程帧
    u16 std_id;  //标准id:0~0x7FF
    u32 ext_id;   //扩展id:0~0x1FFFFFFF
    u8 rx_buf[8];
    u8 dlc;      //长度:0-8
    u8 frame_index;//帧序号
} can_rx_info_TypeDef;


int hw_cane_init(struct cane_config *cane_cfg);
int hw_cane_resume(hw_can_dev can);
int hw_cane_suspend(hw_can_dev can);
int hw_cane_send_buf(enum cane_id_type ide, u32 id, enum cane_tx_rtr_mode rtr, u8 *buf, u8 len);
int hw_cane_send_restart();
int hw_cane_send_stop();
//中断&查询
int hw_cane_get_recv_frame_num();
//中断&查询
int hw_cane_recv_buf(can_rx_info_TypeDef *rx_info);

int hw_cane_baud_cfg(u32 freq);
void hw_cane_filter_rx_cfg(enum cane_rx_filter_mode filter_mode, u32 id_e, u32 idm_e, u32 id_se, u32 idm_se);
int hw_cane_dma_rx_cfg(enum can_work_mode cwm, u8 *dma_addr, u8 dma_rxcnt);
void hw_cane_start();//中途修改配置需调用使能can

void hw_cane_change_callback(void (*cbfun)(enum cane_event, u32));
void hw_cane_set_ie(hw_can_dev can, can_pnd_typedef pnd, u8 en);
u8 hw_cane_get_pnd(hw_can_dev can);//会清空所有pnd

void hw_cane_dump();


#endif

