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

static int _bt_nv_ram_min[BT_NV_RAM_SIZE / 4] sec_used(.sec_bt_nv_ram);//最少占用

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
    return (const char *)bt_cfg.edr_name;
}

extern void bt_ble_init(void);
extern void btstack_task(void);
extern void vPortSuppressTicksAndSleep(u32 usec);
extern void wdt_clear(void);


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

#if (BLE_DUT_TEST || CONFIG_BT_MODE != BT_NORMAL)
    extern u8 bt_get_pwr_max_level(void);
    //PA测试发射功率不能调太大会导致PA坏掉
#if RF_PA_EN
    bt_max_pwr_set(10, 5, 8, SET_BLE_TX_POWER_LEVEL);
#else
    bt_max_pwr_set(10, 5, 8, bt_get_pwr_max_level());
#endif
    user_sele_dut_mode(1);
#else
    bt_max_pwr_set(10, 5, 8, SET_BLE_TX_POWER_LEVEL);
#endif


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

    while (1) {
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

#if (BLE_DUT_TEST || BLE_DUT_API_TEST || CONFIG_BT_MODE != BT_NORMAL)
    extern u8 bt_get_pwr_max_level(void);
    //PA测试发射功率不能调太大会导致PA坏掉
#if RF_PA_EN
    bt_max_pwr_set(10, 5, 8, SET_BLE_TX_POWER_LEVEL);
#else
    bt_max_pwr_set(10, 5, 8, bt_get_pwr_max_level());
#endif
    if (BLE_DUT_TEST) {
        user_sele_dut_mode(SET_DUT_MODE);//设置dut mode
    }
    if (BLE_DUT_API_TEST) {
        user_sele_dut_mode(SET_DUT_API_MODE);
    }
#else
    bt_max_pwr_set(10, 5, 8, SET_BLE_TX_POWER_LEVEL);
#endif


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

static void get_random_number(u8 *ptr, u8 len)
{
    u8 rand_len, tmp_len;
    u32 rand32;
    u8 *tmp_ptr;
    tmp_len = len;
    tmp_ptr = ptr;
    while (tmp_len) {
        rand32 = JL_RAND->R64L;
        rand_len = tmp_len;
        if (tmp_len > 4) {
            rand_len = 4;
        }
        memcpy(tmp_ptr, (u8 *)&rand32, rand_len);
        tmp_ptr += rand_len;
        tmp_len -= rand_len;
        delay_nops(10);
    }
}

static u8 bt_mac_addr_for_testbox[6] = {0};
const u8 *bt_get_mac_addr()
{
    return bt_cfg.mac_addr;
}

void cfg_file_parse(u8 idx)
{
    u8 tmp[128] = {0};
    int ret = 0;

    memset(tmp, 0x00, sizeof(tmp));

    ret = syscfg_read(CFG_BT_NAME, tmp, 32);
    if (32 != ret) {
        log_info("read bt name err or not bt_cfg.bin doc\n");
    } else {
        memset(bt_cfg.edr_name, 0x00, LOCAL_NAME_LEN);
        memcpy(bt_cfg.edr_name, tmp, ret);
    }
    log_info("bt name config:%s\n", bt_cfg.edr_name);

    ret = syscfg_read(CFG_BT_RF_POWER_ID, &bt_cfg.rf_power, 1);
    if (ret != 1) {
        log_debug("read rf err\n");
        bt_cfg.rf_power = 10;
    }
    bt_max_pwr_set(bt_cfg.rf_power, 5, 8, SET_BLE_TX_POWER_LEVEL);
    log_info("rf config:%d\n", bt_cfg.rf_power);
#if (BLE_DUT_TEST || CONFIG_BT_MODE != BT_NORMAL)
    extern u8 bt_get_pwr_max_level(void);
    bt_max_pwr_set(bt_cfg.rf_power, 5, 8, bt_get_pwr_max_level());
#else
    bt_max_pwr_set(bt_cfg.rf_power, 5, 8, SET_BLE_TX_POWER_LEVEL);
    switch (SET_BLE_TX_POWER_LEVEL) {
    case 5:
        log_info("rf config ble power :0 dBm\n");
        break;
    case 6:
        log_info("rf config ble power :4 dBm\n");
        break;
    case 7:
        log_info("rf config ble power :6 dBm\n");
        break;

    default:
        log_info("rf config :%d \n", SET_BLE_TX_POWER_LEVEL);
        break;
    }
#endif

    /*************************************************************************/
    /*                      CFG READ IN VM                                   */
    /*************************************************************************/
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


