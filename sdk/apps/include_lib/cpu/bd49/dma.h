#ifndef _DMA_H_
#define _DMA_H_

#include "typedef.h"

#define IRQ_DMA0_PRIORITY  0
#define IRQ_DMA1_PRIORITY  0
#define IRQ_DMA2_PRIORITY  0
#define IRQ_DMA3_PRIORITY  0



enum dma_index {
    DMA0 = 0,
    DMA1,
    DMA2,
    DMA3,
    DMA_MAX_HW_NUM,
};

//地址自动递增
enum dma_inc {
    DMA_INC_DISABLE = 0,
    DMA_INC_ENABLE,
};

//数据位宽
enum dma_data_size {
    DMA_DATA_SIZE_BYTE = 0,
    DMA_DATA_SIZE_HALFWORD,
    DMA_DATA_SIZE_WORD,
};

//循环模式
enum dma_mode {
    DMA_MODE_NORMAL = 0,
    DMA_MODE_FINITE_CYCLE,
    DMA_MODE_INFINITE_CYCLE,
};

//优先级
enum dma_priority {
    DMA_PRIORITY_LOW = 0,
    DMA_PRIORITY_MEDIUM,
    DMA_PRIORITY_HIGH,
    DMA_PRIORITY_VERY_HIGH,
};

//M2M OR OTHER
enum dma_m2m {
    DMA_M2M_DISABLE = 0,
    DMA_M2M_ENABLE,
};

//EXT MODE
enum dma_ext_mode {
    DMA_EXT_MODE_BIT = 0,       //位封装模式,单位为byte,地址按byte递增(!!!非M2M模式不能使用)
    DMA_EXT_MODE_0_EXPAND,      //0拓展模式,单位为次,每次地址根据源位宽递增地址和目标位宽递增地址
    DMA_EXT_MODE_SIGN_EXPAND,   //符号拓展模式,相当于0拓展模式加符号位
};

//中断类型
enum dma_int_type {
    DMA_INT_TH = 0, //传输过半中断屏蔽/使能
    DMA_INT_TC,     //传输完成中断屏蔽/使能
    DMA_INT_CC,     //循环传输完成中断屏蔽/使能
};

//dma cmd
enum dma_cmd {
    DMA_CMD_DISABLE = 0,//关闭DMA
    DMA_CMD_ENABLE,     //开启DMA
    DMA_CMD_STOP,       //停止DMA传输 - 硬件上会等待当前正在搬运的数据完成,才会停止DMA
    DMA_CMD_GEN_ENABLE, //使能外设通道DMA请求发生器
    DMA_CMD_GEN_DISABLE,//关闭外设通道DMA请求发生器
};

enum dma_gen_pol {
    DMA_GEN_POL_NULL = 0,       //无触发检测和生成
    DMA_GEN_POL_RISING_EDGE,    //触发极性为上升沿
    DMA_GEN_POL_FALLING_EDGE,   //触发极性为下降沿
    DMA_GEN_POL_BOTH_EDGE,      //触发极性为双边沿
};

enum dma_gen_sel0 {
    DMA_GEN_SEL0_DISABLE = 0,
    DMA_GEN_SEL0_IIC0_TASK = BIT(0),
    DMA_GEN_SEL0_IIC0_RESTART = BIT(1),
    DMA_GEN_SEL0_IIC0_STOP = BIT(2),
    DMA_GEN_SEL0_IIC0_RXACK = BIT(3),
    DMA_GEN_SEL0_IIC0_RXNACK = BIT(4),
    DMA_GEN_SEL0_IIC0_ADR_MATCH = BIT(5),
    DMA_GEN_SEL0_IIC0_RXBYTE_DONE = BIT(6),
    DMA_GEN_SEL0_IIC0_TXTASK_LOAD = BIT(7),
    DMA_GEN_SEL0_IIC0_RXTASK_LOAD = BIT(8),
    // #define DMA_GEN_SEL0_    = BIT(9),
    DMA_GEN_SEL0_TMR0_PRD_MATCH = BIT(10),
    DMA_GEN_SEL0_TMR1_PRD_MATCH = BIT(11),
    DMA_GEN_SEL0_TMR2_PRD_MATCH = BIT(12),
    DMA_GEN_SEL0_TMR3_PRD_MATCH = BIT(13),
    DMA_GEN_SEL0_TMR0_CPT = BIT(14),
    DMA_GEN_SEL0_TMR1_CPT = BIT(15),
    DMA_GEN_SEL0_TMR2_CPT = BIT(16),
    DMA_GEN_SEL0_TMR3_CPT = BIT(17),
    DMA_GEN_SEL0_TMR0_PWM = BIT(18),
    DMA_GEN_SEL0_TMR1_PWM = BIT(19),
    DMA_GEN_SEL0_TMR2_PWM = BIT(20),
    DMA_GEN_SEL0_TMR3_PWM = BIT(21),
    DMA_GEN_SEL0_MCPWM0_PWM_H = BIT(22),
    DMA_GEN_SEL0_MCPWM1_PWM_H = BIT(23),
    DMA_GEN_SEL0_MCPWM2_PWM_H = BIT(24),
    DMA_GEN_SEL0_MCPWM0_PWM_L = BIT(25),
    DMA_GEN_SEL0_MCPWM1_PWM_L = BIT(26),
    DMA_GEN_SEL0_MCPWM2_PWM_L = BIT(27),
    DMA_GEN_SEL0_MCPWM0_TMR_OVF = BIT(28),
    DMA_GEN_SEL0_MCPWM1_TMR_OVF = BIT(29),
    DMA_GEN_SEL0_MCPWM2_TMR_OVF = BIT(30),
    DMA_GEN_SEL0_MCPWM0_TMR_ZERO = BIT(31),
};
enum dma_gen_sel1 {
    DMA_GEN_SEL1_DISABLE = 0,
    DMA_GEN_SEL1_MCPWM1_TMR_ZERO = BIT(0),
    DMA_GEN_SEL1_MCPWM2_TMR_ZERO = BIT(1),
    DMA_GEN_SEL1_MCPWM0_FPIN_EDGE = BIT(2),
    DMA_GEN_SEL1_MCPWM1_FPIN_EDGE = BIT(3),
    DMA_GEN_SEL1_MCPWM2_FPIN_EDGE = BIT(4),
    DMA_GEN_SEL1_SARADC_TRI = BIT(5),
    DMA_GEN_SEL1_LPCTM0_DMA_REQ = BIT(6),
};

typedef struct {
    u32 src_addr;                       //源地址,可为内存地址,可为寄存器地址
    u32 dest_addr;                      //目标地址,可为内存地址,可为寄存器地址
    u32 buffersize;                     //设定待传输数据数量(次数),位封装模式为数据量(byte),其他模式下为次数
    enum dma_ext_mode ext_mode;         //DMA数据拓展模式选择
    enum dma_inc src_inc;               //源地址是否使能自动递增
    enum dma_inc dest_inc;              //目标地址是否使能自动递增
    enum dma_data_size src_data_size;   //源数据的数据宽度 8bit /16bit /32bit
    enum dma_data_size dest_data_size;  //目标数据的数据宽度 8bit /16bit /32bit
    enum dma_mode mode;                 //正常为一次传输,可选循环模式
    enum dma_priority priority;         //软件设置通道优先级,4级可选
    enum dma_m2m M2M;                   //传输类型是否为M2M模式
    u16 cycle_time;                     //循环模式时的循环次数
    void (*isr_cb)(enum dma_index channel, enum dma_int_type int_type);//中断回调函数,参数为中断类型,传输一半或者传输完成或者循环完成
    //dma generator cfg
    enum dma_gen_pol gen_pol;           //触发极性选择(传输模式不是M2M时有效)
    u8 gen_req_num;                     //一次触发产生的DMA请求数目,实际上产生的DMA请求数目为req_num+1(1~32)(传输模式不是M2M时有效)
    enum dma_gen_sel0 gen_sel0;          //DMA触发器外设源选择,可与多个外设(传输模式不是M2M时有效)
    enum dma_gen_sel1 gen_sel1;          //DMA触发器外设源选择,可与多个外设(传输模式不是M2M时有效)
} dma_cfg;

int dma_busy_check(enum dma_index channel);
//=================================================================================//
//@brief: 初始化DMA
//@input:
// 		dma: dma的编号;
// 		cfg: 需要配置的参数结构体;
//@return: NULL
//=================================================================================//
int dma_init(enum dma_index channel, dma_cfg *cfg);
//只有调deinit才会释放dma通道,否则保持占用
int dma_deinit(enum dma_index channel);

//=================================================================================//
//@brief: DMA操作命令
//@input:
// 		dma: dma的编号;
// 		cmd: 操作的命令号;
//@return: NULL
//@note: 关闭和停止后要再使用都要重新init;
//@example: 启动DMA传输: dma_cmd(DMA0, DMA_CMD_ENABLE);
//          关闭DMA传输: dma_cmd(DMA0, DMA_CMD_DISABLE);
//          停止DMA传输: dma_cmd(DMA0, DMA_CMD_STOP);
//=================================================================================//
void dma_cmd(enum dma_index channel, enum dma_cmd cmd);

//=================================================================================//
//@brief: dma中断使能/禁能配置
//@input:
// 		dma: dma的编号;
//      dma_int: 操作的中断类型
//      en: 0:关闭对应中断使能 1:打开对应中断使能
//@return: NULL
//@note: 当需要对应中断时,打开对应中断,在初始化之后设置,发生对应中断后,
//       会通过初始化时传入的回调函数通知上层
//@example: 使能DMA0传输完成中断: dma_int_config(DMA0, DMA_INT_TC, 1);
//          使能DMA0传输一半中断: dma_int_config(DMA0, DMA_INT_TH, 1);
//=================================================================================//
void dma_int_config(enum dma_index channel, enum dma_int_type dma_int, u8 en);

//=================================================================================//
//@brief: 查询对应中断pending是否起来了
//@input:
// 		dma: dma的编号;
//      dma_int: 操作的中断类型
//@return: 0: 没有查询到对应中断pending 1: 对应的中断pending起来了
//@note: 使能了对应中断时,就不应该使用查询接口了
//@example: 查询传输一半中断是否起来了: dma_get_pend(DMA0, DMA_INT_TH);
//          查询传输完成中断是否起来了: dma_get_pend(DMA0, DMA_INT_TC);
//=================================================================================//
u8 dma_get_pend(enum dma_index channel, enum dma_int_type dma_int);

//=================================================================================//
//@brief: 死等的方式等待对应中断起来,对应中断起来后返回
//@input:
// 		dma: dma的编号;
//      dma_int: 操作的中断类型
//@return: NULL
//@note: 使能了对应中断时,就不应该使用死等查询接口了
//@example: 等待传输一半: dma_wait_pend(DMA0, DMA_INT_TH);
//          等待传输完成: dma_wait_pend(DMA0, DMA_INT_TC);
//=================================================================================//
void dma_wait_pend(enum dma_index channel, enum dma_int_type dma_int);

//=================================================================================//
//@brief: 获取当前dma传输了多少数据,调用了暂停DMA后可获取
//@input:
// 		dma: dma的编号;
//@return: 当前DMA传输完成了多少数据量(byte)
//@note: DMA传输过程中停止传输,查询传输了多少数据
//@example: dma_cmd(DMA0, DMA_CMD_STOP);//停止当前传输
//          dma_wait_pend(DMA0, DMA_INT_TC);//等待硬件停止
//          copy_len = dma_get_complete_len(DMA0);//获取传输了多少数据
//=================================================================================//
u16 dma_get_complete_len(enum dma_index channel);

//=================================================================================//
//@brief: 获取当前dma传输多少个循环,调用了暂停DMA后可获取,循环模式进入完成中断后可获取
//@input:
// 		dma: dma的编号;
//@return: 当前DMA传输完成了多少个循环
//@note: DMA传输过程中停止传输,或者是起了完成中断,查询传输了多少个循环
//@example: dma_cmd(DMA0, DMA_CMD_STOP);//停止当前传输
//          dma_wait_pend(DMA0, DMA_INT_TC);//等待硬件停止
//          copy_len = dma_get_complete_cycle_time(DMA0);//获取传输了多少个循环
//=================================================================================//
u16 dma_get_complete_cycle_time(enum dma_index channel);

void dma_clear_all_pnd(enum dma_index channel);
int dma_set_sadr_inc(enum dma_index channel, enum dma_inc sadr_inc);
int dma_set_dadr_inc(enum dma_index channel, enum dma_inc dadr_inc);
enum dma_priority dma_get_priority(enum dma_index channel);
int dma_set_priority(enum dma_index channel, enum dma_priority pri);
//当cycle_time=0,不循环
int dma_restart_prepare(enum dma_index channel, u32 saddr, u32 daddr, u16 len, u16 cycle_time);
u8 dma_register_callback(enum dma_index channel, void (*dma_isr_callback)(enum dma_index channel, enum dma_int_type));
void dma_gen_set_sel_trig(enum dma_index channel, enum dma_gen_sel0 sel0, enum dma_gen_sel1 sel1);
#endif /* #ifndef _DMA_H_ */

