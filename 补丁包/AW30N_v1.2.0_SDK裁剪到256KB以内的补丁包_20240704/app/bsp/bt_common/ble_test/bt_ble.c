#include "bt_ble.h"
#include "test_app_config.h"
#include "msg/msg.h"
#include "user_cfg.h"
/* #include "fs.h" */
#include "includes.h"
#include "vm.h"
#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".ble_app_bss")
#pragma data_seg(".ble_app_data")
#pragma const_seg(".ble_app_text_const")
#pragma code_seg(".ble_app_text")
#endif


#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[ble]"
#include "log.h"

#if (TESTE_BLE_EN)

BT_CONFIG bt_cfg = {
    .edr_name        = "BD49_BLE",
    .mac_addr        = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff},
    .tws_local_addr  = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff},
    .rf_power        = 10,
    .dac_analog_gain = 25,
    .mic_analog_gain = 7,
    .tws_device_indicate = 0x6688,
};

const char *bt_get_local_name()
{
    return bt_cfg.edr_name;
}

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

    bt_max_pwr_set(10, 5, 8, SET_BLE_TX_POWER_LEVEL);

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

#if BLE_DUT_TEST
    user_sele_dut_mode(1);
#endif

    bt_max_pwr_set(10, 5, 8, SET_BLE_TX_POWER_LEVEL);

    btstack_init();

#if !BLE_DUT_TEST
#if MASTER
    bt_ble_master_init();
#elif SLAVE
    bt_ble_slave_init();
#endif
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

static u8 bt_mac_addr_for_testbox[6] = {0};
const u8 *bt_get_mac_addr()
{
    int ret = 0;
    u8 mac_buf[6];
    u8 mac_buf_tmp[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    u8 mac_buf_tmp2[6] = {0, 0, 0, 0, 0, 0};

    do {
        ret = syscfg_read(CFG_BT_MAC_ADDR, mac_buf, 6);
        if ((ret != 6) || !memcmp(mac_buf, mac_buf_tmp, 6) || !memcmp(mac_buf, mac_buf_tmp2, 6)) {
            get_random_number(mac_buf, 6);
            syscfg_write(CFG_BT_MAC_ADDR, mac_buf, 6);
        }
    } while (0);

    syscfg_read(CFG_BT_MAC_ADDR, bt_mac_addr_for_testbox, 6);
    if (!memcmp(bt_mac_addr_for_testbox, mac_buf_tmp, 6)) {
        get_random_number(bt_mac_addr_for_testbox, 6);
        syscfg_write(CFG_BT_MAC_ADDR, bt_mac_addr_for_testbox, 6);
        log_info(">>>init mac addr!!!\n");
    }

    log_info("mac:");
    log_info_hexdump(mac_buf, sizeof(mac_buf));
    memcpy(bt_cfg.mac_addr, mac_buf, 6);

    return bt_cfg.mac_addr;
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
void ble_evt_iodebug_in(void)
{
    JL_PORTA->DIR &= ~BIT(2);
    JL_PORTA->OUT |=  BIT(2);
}
void ble_evt_iodebug_out(void)
{
    JL_PORTA->OUT &= ~BIT(2);
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
void ble_evt_iodebug_in(void)
{
}
void ble_evt_iodebug_out(void)
{
}
#endif
