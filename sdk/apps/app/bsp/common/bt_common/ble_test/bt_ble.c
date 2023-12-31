#include "bt_ble.h"
#include "test_app_config.h"
#include "msg/msg.h"

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[ble]"
#include "log.h"

#if (TESTE_BLE_EN)
extern void bt_ble_init(void);
extern void btstack_task(void);
extern void vPortSuppressTicksAndSleep(u32 usec);
extern void wdt_clear(void);
extern void user_sele_dut_mode(bool set);


void testbox_update_msg_handle(int msg);
void ble_module(void)
{
    static u32 i = 0;
    bool temp = 0;

    SFR(JL_CLOCK->PRP_CON1,  0, 3, 1);
    SFR(JL_CLOCK->PRP_CON1, 26, 1, 1);
    SFR(JL_CLOCK->PRP_CON2, 0, 2, 1);   // wl_and_clk 1:std_48m
    SFR(JL_CLOCK->PRP_CON2, 2, 2, 2);   // wl2adc_clk 1:std_48m 2:pll_96m
    SFR(JL_CLOCK->PRP_CON2, 4, 2, 2);   // wl2dac_clk 1:std_48m 2:pll_96m
    SFR(JL_CLOCK->PRP_CON2, 8, 1, 1);   // wl_aud_rst
    asm("csync");

    bt_pll_para(48000000, 48000000, 0, 0);

#if BLE_DUT_TEST
    user_sele_dut_mode(1);
#endif

    extern u8 bt_get_pwr_max_level(void);
    bt_max_pwr_set(10, 5, 8, bt_get_pwr_max_level());

    btstack_init();

#if !BLE_DUT_TEST
#if MASTER
    bt_ble_master_init();
#elif SLAVE
    bt_ble_slave_init();
#endif

    //btstack task 使用软中断执行
    /* extern void btstack_task(void); */
    /* extern const int IRQ_BTSTACK_MSG_IP; */
    /* request_irq(IRQ_SOFT3_IDX, IRQ_BTSTACK_MSG_IP, btstack_task, 0); */
#endif

    while (1) {
#if !BLE_DUT_TEST
        /* btstack_task(); */
#endif
        int msg[2] = {0};
        get_msg(2, &msg[0]);

        testbox_update_msg_handle(msg[0]);

        if (temp == 0) {
            /* if (le_ready_enter_low_power()) { */
            /*     temp = 1; */
            /*     printf("\r\n<<<<<<<<<<<<< ble_enter_low_power >>>>>>>>>>>\r\n"); */
            /* } */
        } else {
            vPortSuppressTicksAndSleep(-2);
            /* if (get_ble_disconnect(2) == 1) { */
            /*     temp = 0; */
            /*     printf("\r\n<<<<<<<<<<<<< ble_exit_low_power >>>>>>>>>>>\r\n"); */
            /* } */
        }

        i++;
#if CONFIG_BLE_CONNECT_SLOT
        //调小间隔后,缩小wdt time
        if (i > 0xfff) {
#else
        if (i > 0xfff) {
#endif
            putchar('~');
            i = 0;
            wdt_clear();
        }
    }
}

/* BLE应用层API接口 */
void bt_init_api(void)
{
    SFR(JL_CLOCK->PRP_CON1,  0, 3, 1);
    SFR(JL_CLOCK->PRP_CON1, 26, 1, 1);
    SFR(JL_CLOCK->PRP_CON2, 0, 2, 1);   // wl_and_clk 1:std_48m
    SFR(JL_CLOCK->PRP_CON2, 2, 2, 2);   // wl2adc_clk 1:std_48m 2:pll_96m
    SFR(JL_CLOCK->PRP_CON2, 4, 2, 2);   // wl2dac_clk 1:std_48m 2:pll_96m
    SFR(JL_CLOCK->PRP_CON2, 8, 1, 1);   // wl_aud_rst
    asm("csync");

    bt_pll_para(48000000, 48000000, 0, 0);

    extern u8 bt_get_pwr_max_level(void);
    bt_max_pwr_set(10, 5, 8, bt_get_pwr_max_level());

    btstack_init();
#if MASTER
    bt_ble_master_init();
#elif SLAVE
    bt_ble_slave_init();
#endif

    //btstack task 使用软中断执行
    /* extern void btstack_task(void); */
    /* extern const int IRQ_BTSTACK_MSG_IP; */
    /* request_irq(IRQ_SOFT3_IDX, IRQ_BTSTACK_MSG_IP, btstack_task, 0); */
}

int get_buffer_vaild_len(void *priv)
{
    u32 vaild_len = 0;
    ble_op_att_get_remain(&vaild_len);
    return vaild_len;
}
#else
void ble_module(void)
{

}
#endif

#if 0   //蓝牙底层状态io debug
void ble_rx_irq_iodebug_in(void)
{
    JL_PORTA->DIR &= ~BIT(0);
    JL_PORTA->OUT |=  BIT(0);
}
void ble_rx_irq_iodebug_out(void)
{
    JL_PORTA->OUT &= ~BIT(0);
}
void btstack_iodebug_in(void)
{
    JL_PORTA->DIR &= ~BIT(1);
    JL_PORTA->OUT |=  BIT(1);
}
void btstack_iodebug_out(void)
{
    JL_PORTA->OUT &= ~BIT(1);
}
#else
void ble_rx_irq_iodebug_in(void)
{
}
void ble_rx_irq_iodebug_out(void)
{
}
void btstack_iodebug_in(void)
{
}
void btstack_iodebug_out(void)
{
}
#endif
