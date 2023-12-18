#include "code_v2/update.h"
#include "code_v2/update_loader_download.h"
#include "crc16.h"
#include "wdt.h"
/* #include "os/os_api.h" */
/* #include "os/os_cpu.h" */
#include "app_config.h"
#include "cpu.h"
/* #include "syscfg_id.h" */
#include "btcontroller_modules.h"
/* #include "system/includes.h" */
#include "code_v2/uart_update.h"
#include "code_v2/dual_bank_updata_api.h"
/* #include "btstack/avctp_user.h" */
/* #include "poweroff.h" */
/* #include "app_main.h" */
#include "vm.h"
#include "clock.h"

/* #if TCFG_USER_TWS_ENABLE */
/* #include "bt_tws.h" */
/* #endif */
/*  */
/* #if TCFG_UI_ENABLE */
/* #include "ui/ui_api.h" */
/* #endif */

#ifdef UPDATE_VOICE_REMIND
#include "app_tone.h"
#include "audio_config.h"
#endif

#ifdef UPDATE_LED_REMIND
#include "asm/pwm_led_hw.h"
#endif

/* #include "custom_cfg.h" */

#define LOG_TAG_CONST       UPDATE
//#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[APP-UPDATE]"
#include "log.h"

/* #if (JL_RCSP_EXTRA_FLASH_OPT) */
/* #include "rcsp_extra_flash_opt.h" */
/* #endif */




/////////////////////////////undef macro/////////////////////////
#define TCFG_UI_ENABLE 0
#define CONFIG_DEBUG_ENABLE 0
#define CONFIG_UPDATE_JUMP_TO_MASK 0
#define TCFG_BT_BACKGROUND_ENABLE 0
#define TCFG_AUTO_SHUT_DOWN_TIME 0
#define OTA_TWS_SAME_TIME_ENABLE 0
#define TCFG_APP_BT_EN 0
#define TCFG_AUDIO_ANC_ENABLE 0
/////////////////////////////////////////////////////////////////

#define LOADER_NAME		"LOADER.BIN"
#define DEVICE_UPDATE_KEY_ERR  BIT(30)
#define DEVICE_FIRST_START     BIT(31)

extern void update_module_init(void (*cbk)(update_mode_info_t *, u32, void *));
extern void testbox_update_init(void);
extern void ll_hci_destory(void);
extern void hci_controller_destory(void);
extern void ram_protect_close(void);
extern void hwi_all_close(void);
extern void wifi_det_close();

__attribute__((weak))
void wifi_det_close()
{
    printf("tmp weak func wifi_det_close\n");
}
extern u8 get_ota_status();
extern int get_nor_update_param(void *buf);
extern bool get_tws_phone_connect_state(void);
extern void tws_sniff_controle_check_disable(void);
extern void sys_auto_sniff_controle(u8 enable, u8 *addr);
extern void app_audio_set_wt_volume(s16 volume);
extern u8 get_max_sys_vol(void);
extern u8 get_bt_trim_info_for_update(u8 *res);


extern const int support_norflash_update_en;
/* const u8 loader_file_path[] = "mnt/norflash/C/"LOADER_NAME""; */
//升级文件路径必须是短文件名（8+3）结构，仅支持２层目录
/* const char updata_file_name[] = "/UPDATA/JL_692X.BFU"; */
const char updata_file_name[] = "/*.UFW";
static u32 g_updata_flag = 0;
static volatile u8 ota_status = 0;
static succ_report_t succ_report;

bool vm_need_recover(void)
{
    printf(">>>[test]:g_updata_flag = 0x%x\n", g_updata_flag);
    return ((g_updata_flag & 0xffff) == UPDATA_SUCC) ? true : false;
}


u16 update_result_get(void)
{
    u16 ret = UPDATA_NON;

    if (CONFIG_UPDATE_ENABLE) {
        UPDATA_PARM *p = UPDATA_FLAG_ADDR;
        u16 crc_cal;
        crc_cal = CRC16(((u8 *)p) + 2, sizeof(UPDATA_PARM) - 2);	//2 : crc_val
        if (crc_cal && crc_cal == p->parm_crc) {
            ret =  p->parm_result;
        }
        g_updata_flag = ret;
        g_updata_flag |= ((u32)(p->magic)) << 16;

        memset(p, 0x00, sizeof(UPDATA_PARM));
    }

    return ret;
}

void update_result_set(u16 result)
{
    if (CONFIG_UPDATE_ENABLE) {
        UPDATA_PARM *p = UPDATA_FLAG_ADDR;

        memset(p, 0x00, sizeof(UPDATA_PARM));
        p->parm_result = result;
        p->parm_crc = CRC16(((u8 *)p) + 2, sizeof(UPDATA_PARM) - 2);
    }
    /* #if (RCSP_UPDATE_EN && RCSP_MODE && JL_RCSP_EXTRA_FLASH_OPT) */
    /*     if (UPDATA_SUCC == result) { */
    /*         rcsp_eflash_update_flag_set(0); */
    /*         rcsp_eflash_flag_set(0); */
    /*         extern void set_update_ex_flash_flag(u8 update_flag); */
    /*         set_update_ex_flash_flag(0); */
    /*     } */
    /* #endif */
}
void update_clear_result()
{
    g_updata_flag = 0;
}

bool update_success_boot_check(void)
{
    if (CONFIG_UPDATE_ENABLE) {
        u16 result = g_updata_flag & 0xffff;
        u16 up_tpye = g_updata_flag >> 16;
        if ((UPDATA_SUCC == result) && ((SD0_UPDATA == up_tpye) || (SD1_UPDATA == up_tpye) || (USB_UPDATA == up_tpye))) {
            return true;
        }
    }
    return false;
}

bool device_is_first_start()
{
    log_info("g_updata_flag=0x%x\n", g_updata_flag);
    if ((g_updata_flag & DEVICE_FIRST_START) || (g_updata_flag & DEVICE_UPDATE_KEY_ERR) || (g_updata_flag == UPDATA_SUCC)) {
        log_info("\n=================device_is_first_start=========================\n");
        return true;
    }
    return false;
}

void led_update_start(void)
{
#ifdef UPDATE_LED_REMIND
    puts("led_update_start\n");
    /* pwm_led_mode_set(PWM_LED_ALL_OFF); */
#endif
}

void led_update_finish(void)
{
#ifdef UPDATE_LED_REMIND
    puts("led_update_finish\n");
    /* pwm_led_mode_set(PWM_LED0_LED1_FAST_FLASH); */
#endif
}

static inline void dev_update_close_ui()
{

#if (TCFG_UI_ENABLE&&(CONFIG_UI_STYLE == STYLE_JL_LED7))
    u8 count = 0;
    UI_SHOW_WINDOW(ID_WINDOW_POWER_OFF);
__retry:
    if (UI_GET_WINDOW_ID() != ID_WINDOW_POWER_OFF) {
        os_time_dly(10);//增加延时防止没有关显示
        count++;
        if (count < 3) {
            goto __retry;
        }
    }
#endif
}

int update_result_deal()
{
    /* #ifdef CONFIG_FPGA_ENABLE */
    /*     return 0; */
    /* #endif */

    u8 key_voice_cnt = 0;
    u16 result = 0;
    result = (g_updata_flag & 0xffff);
    log_info("<--------update_result_deal=0x%x %x--------->\n", result, g_updata_flag >> 16);
#if CONFIG_DEBUG_ENABLE
#if TCFG_APP_BT_EN
    u8 check_update_param_len(void);
    ASSERT(check_update_param_len(), "UPDATE_PARAM_LEN ERROR");
#endif
#endif
    if (result == UPDATA_NON || 0 == result) {
        return 0;
    }
    if (result == UPDATA_SUCC) {
        /* #if((RCSP_MODE == RCSP_MODE_EARPHONE) && RCSP_UPDATE_EN) */
        /*         u8 clear_update_flag = 0; */
        /*         syscfg_write(VM_UPDATE_FLAG, &clear_update_flag, 1); */
        /* #endif */
#ifdef UPDATE_LED_REMIND
        led_update_finish();
#endif
    }

    int voice_max_cnt = 5;
    while (1) {
        wdt_clear();
        key_voice_cnt++;
#ifdef UPDATE_VOICE_REMIND
        extern void update_tone_event_clear();
        if (result == UPDATA_SUCC) {
            puts("<<<<<<UPDATA_SUCC");
            /* app_audio_set_volume(APP_AUDIO_STATE_WTONE, get_max_sys_vol() / 2, 1); */
            play_tone_file(get_tone_files()->normal);
            os_time_dly(25);
            puts(">>>>>>>>>>>\n");
            update_tone_event_clear();
        } else {
            voice_max_cnt = 20; //区分下升级失败提示音
            log_info("!!!!!!!!!!!!!!!updata waring !!!!!!!!!!!=0x%x\n", result);
            /* app_audio_set_volume(APP_AUDIO_STATE_WTONE, get_max_sys_vol(), 1); */
            play_tone_file(get_tone_files()->normal);
            os_time_dly(10);
            update_tone_event_clear();
        }
#endif
        if (key_voice_cnt > voice_max_cnt) {
            key_voice_cnt = 0;
            log_info("enter_sys_soft_poweroff\n");
            break;
            //注:关机要慎重,要设置开机键
            //enter_sys_soft_poweroff();
        }
    }

    return 1;
}

void clr_update_ram_info(void)
{
    UPDATA_PARM *p = UPDATA_FLAG_ADDR;
    memset(p, 0x00, sizeof(UPDATA_PARM));
}

void update_close_hw(void *filter_name)
{
    const struct update_target *p;
    list_for_each_update_target(p) {
        if (memcmp(filter_name, p->name, strlen(filter_name)) != 0) {
            printf("close Hw Name : %s\n", p->name);
            p->driver_close();
        }
    }
}

static void update_before_jump_common_handle(UPDATA_TYPE up_type)
{

#if TCFG_AUDIO_ANC_ENABLE
    extern void audio_anc_hw_close();
    audio_anc_hw_close();
#endif

#if (CPU_CORE_NUM == 1)         //双核在跳转前关中断lock_set后会和maskrom 初始化中断冲突导致ilock err
    local_irq_disable();
#endif

    hwi_all_close();

    /* #ifdef CONFIG_SUPPORT_WIFI_DETECT */
    /*     wifi_det_close(); */
    /* #endif */
    /*跳转的时候遇到死掉的情况很可能是硬件模块没关导致，加上保护可以判断哪个异常，保护的地址根据不同SDK而定*/
    /* u8 inv = 0; */
    /* mpu_set(1, (u32)&test_pro_addr, (u32)test_pro_addr, inv, "0r", DBG_FM); */

}

//ota.bin 放到exflash升级的方式，parm_priv存放了norflash的参数，对应实际升级方式的参数需要放在norflash参数之后
void update_param_priv_fill(UPDATA_PARM *p, void *priv, u16 priv_len)
{
    int parm_offset = 0;
    if (support_norflash_update_en) {
        parm_offset = get_nor_update_param(p->parm_priv);          //如果loader放在外挂norflash,parm_priv前面放norflash参数，后面才是升级类型本身的参数
    }
    memcpy(p->parm_priv + parm_offset, priv, priv_len);
}

void update_param_ext_fill(UPDATA_PARM *p, u8 ext_type, u8 *ext_data, u8 ext_len)
{
    struct ext_arg_t ext_arg;

    ext_arg.type = ext_type;
    ext_arg.len  = ext_len;
    ext_arg.data = ext_data;

    ASSERT(((u32)p + sizeof(UPDATA_PARM) + p->ext_arg_len + ext_len + 2 - (u32)p) <= UPDATA_PARM_SIZE, "update param overflow!\n");
    memcpy((u8 *)p + sizeof(UPDATA_PARM) + p->ext_arg_len, &ext_arg, 2);          //2byte:type + len
    memcpy((u8 *)p + sizeof(UPDATA_PARM) + p->ext_arg_len + 2, ext_arg.data, ext_arg.len);
    log_info("ext_fill :");
    log_info_hexdump((u8 *)p + sizeof(UPDATA_PARM) + p->ext_arg_len, ext_arg.len + 2);
    p->ext_arg_len += (2 + ext_arg.len);
    p->ext_arg_crc = CRC16((u8 *)p + sizeof(UPDATA_PARM), p->ext_arg_len);
}

//fill common content \ private content \ crc16
static void update_param_content_fill(int type, UPDATA_PARM *p, void (*priv_param_fill_hdl)(UPDATA_PARM *P))
{
    u8 ext_len = 0;
    u8 *ext_data = NULL;

    memset((u8 *)p, 0x00, sizeof(UPDATA_PARM));

    if (support_norflash_update_en) {
        p->parm_type = NORFLASH_UPDATA;                                //uboot通过该标识从外挂flash读取ota.bin
        *((u16 *)((u8 *)p + sizeof(UPDATA_PARM) + 32)) = (u16)type;    //将实际的升级类型保存到UPDATA_PARM后
    } else {
        p->parm_type = (u16)type;
    }

    p->parm_result = (u16)UPDATA_READY;
    p->magic = UPDATE_PARAM_MAGIC;
    p->ota_addr = succ_report.loader_saddr;

    //支持loader放到外挂flash里ota_addr为0
    if (0 == p->ota_addr && !support_norflash_update_en) {
        log_error("ota addr err\n");
        return;
    }

    if (priv_param_fill_hdl) {
        priv_param_fill_hdl(p);
    }

#if (defined CONFIG_CPU_BR36 || defined CONFIG_CPU_BR27)
    if (type == BT_UPDATA || type == BLE_APP_UPDATA || type == SPP_APP_UPDATA || type == BLE_TEST_UPDATA) {     //D版芯片蓝牙相关的升级需要保存LDO_TRIM_RES
        ext_data = malloc(256);
        if (ext_data != NULL) {
            ext_len = get_bt_trim_info_for_update(ext_data);
            printf("ext_len:%d\n", ext_len);
            update_param_ext_fill(p, EXT_LDO_TRIM_RES, ext_data, ext_len);
            free(ext_data);
        }
    }
#endif
    u8 ext_flag = 0;
    ext_len = 1;
#if CONFIG_UPDATE_JUMP_TO_MASK
    ext_flag = 1;
#endif
    update_param_ext_fill(p, EXT_JUMP_FLAG, &ext_flag, ext_len);

    p->parm_crc = CRC16(((u8 *)p) + 2, sizeof(UPDATA_PARM) - 2);	//2 : crc_val
}

static void update_param_ram_set(u8 *buf, u16 len)
{
    u8 *update_ram = UPDATA_FLAG_ADDR;
    memcpy(update_ram, (u8 *)buf, len);
}

void update_mode_api_v2(UPDATA_TYPE type, void (*priv_param_fill_hdl)(UPDATA_PARM *p), void (*priv_update_jump_handle)(int type))
{
    u16 update_param_len = UPDATA_PARM_SIZE;//sizeof(UPDATA_PARM) + UPDATE_PRIV_PARAM_LEN;

    u8 parm[UPDATA_PARM_SIZE] = {0};
    UPDATA_PARM *p = &parm;

    if (p) {
        update_param_content_fill(type, p, priv_param_fill_hdl);

        if (succ_report.update_param_write_hdl) {
            succ_report.update_param_write_hdl(succ_report.priv_param, (u8 *)p, update_param_len);
        }

#ifdef UPDATE_LED_REMIND
        led_update_start();
#endif

        update_param_ram_set((u8 *)p, sizeof(UPDATA_PARM));
        if (type == BT_UPDATA) {
#if TCFG_BT_BACKGROUND_ENABLE
            if (!app_in_mode(APP_MODE_BT)) {
                //这里需要切到蓝牙模式再切时钟，如果再Music等模式需要较高主频才能跑得过来的任务，切换时钟会导致系统跑不过来导致异常
                g_bt_hdl.background.backmode = BACKGROUND_GOBACK_WITH_OTHER;
                app_send_message(APP_MSG_GOTO_MODE, APP_MODE_BT);
                while (!app_in_mode(APP_MODE_BT)) {
                    os_time_dly(1);
                }
            }
#endif
            /* clk_set_api("sys", 24000000);       //测试盒跳转升级需要强制设置时钟为24M */
            clk_set("sys", 24000000);       //测试盒跳转升级需要强制设置时钟为24M
        }


#if CPU_CORE_NUM > 1            //双核需要把CPU1关掉
        printf("Before Suspend Current Cpu ID:%d Cpu In Irq?:%d\n", current_cpu_id(),  cpu_in_irq());
        if (current_cpu_id() == 1) {
            os_suspend_other_core();
        }
        ASSERT(current_cpu_id() == 0);          //确保跳转前CPU1已经停止运行
        cpu_suspend_other_core(CPU_SUSPEND_TYPE_UPDATE);
        printf("After Suspend Current Cpu ID:%d\n", current_cpu_id());
#endif
        update_before_jump_common_handle(type);

        if (priv_update_jump_handle) {
            priv_update_jump_handle(type);
        }
        /* free(p); */
    } else {
        ASSERT(p, "malloc update param err \n");
    }
}

int update_check_sniff_en(void)

{
    if (CONFIG_UPDATE_ENABLE) {
        if (get_ota_status()) {
            log_info("ota ing...");
            return 0;
        } else {
            return 1;
        }
    }
    return 1;
}


void set_ota_status(u8 stu)
{
    ota_status = stu;
}

u8 get_ota_status()
{
    return ota_status;
}

static u8 ota_idle_query(void)
{
    return !ota_status;
}

//防止升级过程进入powerdown
/* REGISTER_LP_TARGET(ota_lp_target) = { */
/*     .name = "ota", */
/*     .is_idle = ota_idle_query, */
/* }; */

extern void tws_sync_update_api_register(const update_op_tws_api_t *op);
extern update_op_tws_api_t *get_tws_update_api(void);

extern const int support_dual_bank_update_en;
extern int tws_ota_init(void);
extern void tws_api_auto_role_switch_disable();
extern void tws_api_auto_role_switch_enable();

static void update_init_common_handle(int type)
{
    ota_status = 1;
    if (UPDATE_DUAL_BANK_IS_SUPPORT()) {
#if TCFG_AUTO_SHUT_DOWN_TIME
        sys_auto_shut_down_disable();
#endif

#if OTA_TWS_SAME_TIME_ENABLE
        tws_api_auto_role_switch_disable();
        tws_sync_update_api_register(get_tws_update_api());
        tws_ota_init();
#endif
    }
}

static void update_exit_common_handle(int type, void *priv)
{
    update_ret_code_t *ret_code = (update_ret_code_t *)priv;

#if TCFG_AUTO_SHUT_DOWN_TIME
    sys_auto_shut_down_enable();
#endif

#if OTA_TWS_SAME_TIME_ENABLE
    if (UPDATE_DUAL_BANK_IS_SUPPORT()) {
        tws_api_auto_role_switch_enable();
    }
#endif

    ota_status = 0;
}

static void update_common_state_cbk(update_mode_info_t *info, u32 state, void *priv)
{
    int type = info->type;

    log_info("type:%x state:%x code:%x\n", type, state, priv);

    switch (state) {
    case UPDATE_CH_INIT:
        //如果开启了VM配置项暂存RAM功能则在每次触发升级前保存数据到vm_flash
        /* if (get_vm_ram_storage_enable()) { */
        /*     vm_flush2flash(); */
        /* } */

        memset((u8 *)&succ_report, 0x00, sizeof(succ_report_t));
        update_init_common_handle(info->type);
        dev_update_close_ui();
        break;

    case UPDATE_CH_SUCESS_REPORT:
        log_info("succ report stored\n");
        memcpy((u8 *)&succ_report, (u8 *)priv, sizeof(succ_report_t));
        break;
    }

    if (info->state_cbk) {
        info->state_cbk(type, state, priv);
    }

    switch (state) {
    case UPDATE_CH_EXIT:
        update_exit_common_handle(info->type, priv);
        break;
    }
}


int app_update_init(void)
{
    update_module_init(update_common_state_cbk);
    testbox_update_init();
    return 0;
}

/* __initcall(app_update_init); */


void update_start_exit_sniff(void)
{
#if TCFG_APP_BT_EN
#if TCFG_USER_TWS_ENABLE
    if (tws_api_get_tws_state() & TWS_STA_PHONE_CONNECTED) {
        g_printf("exit sniff mode...\n");
        bt_cmd_prepare(USER_CTRL_ALL_SNIFF_EXIT, 0, NULL);
    } else {
        tws_api_tx_unsniff_req();
    }
    tws_sniff_controle_check_disable();
#else
    bt_cmd_prepare(USER_CTRL_ALL_SNIFF_EXIT, 0, NULL);
#endif
    sys_auto_sniff_controle(0, NULL);
#endif
}

#include "app_modules.h"
#include "bt_ble.h"
#include "test_app_config.h"
#include "msg.h"
#if CONFIG_APP_OTA_EN
#include "rcsp_user_update.h"
#endif
#if TESTBOX_UART_UPDATE_EN
#include "testbox_uart_update.h"
#endif
#include "audio.h"
#include "adc_api.h"

void update_cb_register_init(void)
{
    audio_off();
    adc_hw_uninit();
}
void (*update_enter_callback)(void) = update_cb_register_init;
void update_enter_cb_register(void *callback)
{
    update_enter_callback = callback;
}
#if TRANS_DATA_HID_EN || (TESTE_BLE_EN && SLAVE)
extern int get_ble_connect_handle(void);
#endif
void app_update_handle(int msg)
{
    int connect_handle = 0;
    switch (msg) {
#if TCFG_USER_BLE_ENABLE
    /* 蓝牙测试盒升级 */
    case MSG_BLE_TESTBOX_UPDATE_START:
        testbox_update_ble_handle();
        break;
#endif

#if CONFIG_APP_OTA_EN
    /* 手机APP & usb_dongle从机升级 */
    case MSG_BLE_APP_UPDATE_START:
#if TRANS_DATA_HID_EN || (TESTE_BLE_EN && SLAVE)
        connect_handle = get_ble_connect_handle();
        ble_op_latency_close(connect_handle);
#endif
        app_ota_update_handle();
        break;
#endif
#if TESTBOX_UART_UPDATE_EN
    case MSG_UART_TESTBOX_UPDATE_START:
        /* 测试盒串口升级 */
        uart_update_ota_loop(NULL, 0);
        break;
#endif
    }

}

