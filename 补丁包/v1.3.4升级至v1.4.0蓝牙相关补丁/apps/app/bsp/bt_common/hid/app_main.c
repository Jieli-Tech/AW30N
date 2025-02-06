/*********************************************************************************************
    *   Filename        : app_main.c

    *   Description     :

    *   Author          : Wangzhongqiang by Bingquan

    *   Email           : caibingquan@zh-jieli.com

    *   Last modifiled  : 2023-08-29 14:54

    *   Copyright:(c)JIELI  2023-2023  @ , All Rights Reserved.
*********************************************************************************************/
#include "includes.h"
#include "hid_app_config.h"
#include "app_action.h"
#include "app_main.h"
/* #include "update.h" */
/* #include "update_loader_download.h" */
/* #include "app_charge.h" */
/* #include "app_power_manage.h" */
/* #include "asm/charge.h" */

#if (TRANS_DATA_HID_EN)

#define LOG_TAG_CONST       APP
#define LOG_TAG             "[APP]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE
#include "log.h"
/* #include "debug.h" */


#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".ble_app_bss")
#pragma data_seg(".ble_app_data")
#pragma const_seg(".ble_app_text_const")
#pragma code_seg(".ble_app_text")
#endif


static int _bt_nv_ram_min[BT_NV_RAM_SIZE / 4] sec_used(.sec_bt_nv_ram);//最少占用

struct application *main_application_operation_state(char *name, u8 name_len, struct application *app, enum app_state state, struct intent *it);

#if 0
/*任务列表 */
const struct task_info task_info_table[] = {
    {"app_core",            1,     0,   640,   128  },
    {"sys_event",           7,     0,   256,   0    },
    {"btctrler",            4,     0,   512,   256  },
    {"btencry",             1,     0,   512,   128  },
    {"btstack",             3,     0,   768,   256  },
    {"systimer",		    7,	   0,   128,   0    },
    {"update",				1,	   0,   512,   0    },
#if (RCSP_BTMATE_EN)
    {"rcsp_task",		    2,	   0,    640,	0   },
#endif
#if TCFG_AUDIO_ENABLE
    {"audio_dec",           3,     0,   768,   128  },
    {"audio_enc",           4,     0,   512,   128  },
    {"aec",                 2,     0,   768,   128  },
#endif/*TCFG_AUDIO_ENABLE*/
#if TCFG_KWS_VOICE_RECOGNITION_ENABLE
    {"kws",                 2,     0,   256,   64   },
#endif /* #if TCFG_KWS_VOICE_RECOGNITION_ENABLE */

    {0, 0},
};
#endif

APP_VAR app_var;

void app_var_init(void)
{
    app_var.play_poweron_tone = 1;

    app_var.auto_off_time =  0; //TCFG_AUTO_SHUT_DOWN_TIME;
    app_var.warning_tone_v = 340;
    app_var.poweroff_tone_v = 330;
}

__attribute__((weak))
u8 get_charge_online_flag(void)
{
    return 0;
}

void clr_wdt(void);
#if 0//TCFG_POWER_ON_NEED_KEY
void check_power_on_key(void)
{

    u32 delay_10ms_cnt = 0;
    while (1) {
        clr_wdt();
        os_time_dly(1);

        extern u8 get_power_on_status(void);
        if (get_power_on_status()) {
            log_info("+");
            delay_10ms_cnt++;
            if (delay_10ms_cnt > 70) {
                /* extern void set_key_poweron_flag(u8 flag); */
                /* set_key_poweron_flag(1); */
                return;
            }
        } else {
            log_info("-");
            delay_10ms_cnt = 0;
            log_info("enter softpoweroff\n");
            power_set_soft_poweroff();
        }
    }
}
#endif


void hid_app_main()
{
    struct intent it;

    //TODO
    /* if (!UPDATE_SUPPORT_DEV_IS_NULL()) { */
    /*     int update = 0; */
    /*     update = update_result_deal(); */
    /* } */

    printf(">>>>>>>>>>>>>>>>> hid app_main...\n");

    if (get_charge_online_flag()) {
#if 0//(TCFG_SYS_LVD_EN == 1)
        vbat_check_init();
#endif
    } else {
        //TODO
        /* extern void check_power_on_voltage(void); */
        /* check_power_on_voltage(); */
    }

#if 0//TCFG_POWER_ON_NEED_KEY
    check_power_on_key();
#endif

#if 0//TCFG_AUDIO_ENABLE
    //TODO
    /* extern int audio_dec_init(); */
    /* extern int audio_enc_init(); */
    /* audio_dec_init(); */
    /* audio_enc_init(); */
#endif/*TCFG_AUDIO_ENABLE*/

#if 0//TCFG_KWS_VOICE_RECOGNITION_ENABLE
    jl_kws_main_user_demo();
#endif /* #if TCFG_KWS_VOICE_RECOGNITION_ENABLE */

#if TCFG_CHARGE_ENABLE
    set_charge_event_flag(1);
#endif

    init_intent(&it);

#if(CONFIG_APP_KEYBOARD)
    log_info("CONFIG_APP_KEYBOARD");
    it.name = "hid_key";
    it.action = ACTION_HID_MAIN;

#else
    while (1) {
        printf("no app!!!");
    }
#endif

    log_info("run app>>> %s", it.name);
    log_info("%s,%s", __DATE__, __TIME__);

    main_application_operation_state((char *)it.name, sizeof(it.name), NULL, APP_STA_START, &it);

}

/*
 * app模式切换
 */
void app_switch(const char *name, int action)
{
#if 0
    struct intent it;
    struct application *app;

    log_info("app_exit\n");

    init_intent(&it);
    app = get_current_app();
    if (app) {
        /*
         * 退出当前app, 会执行state_machine()函数中APP_STA_STOP 和 APP_STA_DESTORY
         */
        it.name = app->name;
        it.action = ACTION_BACK;
        start_app(&it);
    }

    /*
     * 切换到app (name)并执行action分支
     */
    it.name = name;
    it.action = action;
    start_app(&it);
#endif
}

int eSystemConfirmStopStatus(void)
{
    /* 系统进入在未来时间里，无任务超时唤醒，可根据用户选择系统停止，或者系统定时唤醒(100ms) */
    //1:Endless Sleep
    //0:100 ms wakeup
    /* log_info("100ms wakeup"); */
    return 1;
}

__attribute__((used)) int *__errno()
{
    static int err;
    return &err;
}

struct application *main_application_operation_state(char *name, u8 name_len, struct application *app, enum app_state state, struct intent *it)
{
    const struct application *dev = NULL;
    list_for_each_app_main(dev) {
        if (memcmp(dev->name, name, name_len) == 0) {
            if (dev->ops) {
                dev->ops->state_machine(app, state, it);
            }
        } else {
            printf("no app run");
        }
    }

    return NULL;
}

struct application *main_application_operation_event(char *name, u8 name_len, struct application *app, struct sys_event *event)
{
    const struct application *dev = NULL;
    list_for_each_app_main(dev) {
        if (memcmp(dev->name, name, name_len) == 0) {
            if (dev->ops) {
                dev->ops->event_handler(app, event);
            }
        } else {
            printf("no event run");
        }
    }

    return NULL;
}
#else
void hid_app_main()
{

}
#endif

