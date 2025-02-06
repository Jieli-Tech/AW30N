#include "code_v2/update.h"
#include "code_v2/update_loader_download.h"
#include "btctrler_task.h"
#include "app_config.h"
#include "clock.h"
#include "msg/msg.h"


#if defined(CONFIG_SPP_AND_LE_CASE_ENABLE) || defined(CONFIG_HID_CASE_ENABLE)
#include "lib_profile_cfg.h"
#else
#include "bt_profile_cfg.h"
#endif

#define LOG_TAG "[TEST-UPDATE]"
#define LOG_INFO_ENABLE
#define LOG_ERROR_ENABLE
#include "log.h"

extern u8 check_le_conn_disconnet_flag(void);
extern void ble_app_disconnect(void);
extern void ll_hci_destory(void);
extern void ram_protect_close(void);
extern void update_close_hw(void *filter_name);
extern void btctrler_testbox_update_msg_handle_register(void (*handle)(int));
extern void __bt_updata_reset_bt_bredrexm_addr(void);
extern int __bt_updata_save_connection_info(void);
extern const update_op_api_t lmp_ch_update_op;
extern const update_op_api_t ble_ll_ch_update_op;
u8 ble_update_ready_jump_flag = 0;

static void testbox_bt_classic_update_private_param_fill(UPDATA_PARM *p)
{

}

static void testbox_bt_classic_update_before_jump_handle(int type)
{
}

static void testbox_bt_classic_update_state_cbk(int type, u32 state, void *priv)
{
}

u8 ble_update_get_ready_jump_flag(void)
{
    return ble_update_ready_jump_flag;
}

static void testbox_ble_update_private_param_fill(UPDATA_PARM *p)
{
    u8 addr[6];
    extern int le_controller_get_mac(void *addr);
    le_controller_get_mac(addr);
    //memcpy(p->parm_priv, addr, 6);
    update_param_priv_fill(p, addr, sizeof(addr));

#if UPDATA_KEEP_IO_ENABLE
    update_keep_io(p);
#endif
}

static void testbox_ble_update_before_jump_handle(int type)
{
    ll_hci_destory();

    common_update_before_jump_reset_handle();
}

static void testbox_ble_update_state_cbk(int type, u32 state, void *priv)
{
    update_ret_code_t *ret_code = (update_ret_code_t *)priv;

    if (ret_code) {
        log_error("ret_code->stu:%d err_code:%d\n", ret_code->stu, ret_code->err_code);
    }

    switch (state) {
    case UPDATE_CH_EXIT:
        if (UPDATE_DUAL_BANK_IS_SUPPORT()) {
            if ((0 == ret_code->stu) && (0 == ret_code->err_code)) {
                log_info("bt update succ\n");
                update_result_set(UPDATA_SUCC);
            } else {
                log_info("bt update fail\n");
                update_result_set(UPDATA_DEV_ERR);
            }
        } else {
            if ((0 == ret_code->stu) && (0 == ret_code->err_code)) {
#if TCFG_USER_BLE_ENABLE && (TCFG_BLE_DEMO_SELECT != DEF_BLE_DEMO_NULL) \
        &&(TCFG_BLE_DEMO_SELECT != DEF_BLE_DEMO_ADV || defined(CONFIG_MESH_CASE_ENABLE))\
		&& (TCFG_BLE_DEMO_SELECT != DEF_BLE_DEMO_CLIENT)

                ble_update_ready_jump_flag = 1;
                /* ble_app_disconnect(); */
                extern void ble_module_enable(u8 en);
                ble_module_enable(0);
                u8 cnt = 0;
                while (!check_le_conn_disconnet_flag()) {
                    log_info("wait discon\n");
                    if (cnt++ > 5) {
                        break;
                    }
                }

                //update_mode_api(BLE_TEST_UPDATA);
                update_mode_api_v2(BLE_TEST_UPDATA,
                                   testbox_ble_update_private_param_fill,
                                   testbox_ble_update_before_jump_handle);
#endif
            } else {
#if UPDATA_KEEP_IO_ENABLE
                log_info("update fail, latch reset!!!\n");
                update_get_latch_romio(NULL);
                common_update_before_jump_reset_handle();
#else
                log_info("update fail, cpu reset!!!\n");
                system_reset(UPDATE_FLAG);
#endif
            }
        }
        break;
    }
}
void testbox_update_ble_handle(void)
{
    if (!CONFIG_UPDATE_TESTBOX_BLE_EN) {
        return;
    }
    /* 蓝牙测试盒升级 */
    update_mode_info_t info = {
        .type = BLE_TEST_UPDATA,
        .state_cbk =  testbox_ble_update_state_cbk,
        .p_op_api = &ble_ll_ch_update_op,
        .task_en = 1,
    };
    app_active_update_task_init(&info);
}

void testbox_update_msg_handle(int msg)
{
    /* log_info("msg:%x\n", msg); */

    switch (msg) {
#if TCFG_USER_BLE_ENABLE
    case MSG_BLE_TESTBOX_UPDATE_START: {
        update_mode_info_t info = {
            .type = BLE_TEST_UPDATA,
            .state_cbk =  testbox_ble_update_state_cbk,
            .p_op_api = &ble_ll_ch_update_op,
            .task_en = 1,
        };
        app_active_update_task_init(&info);
    }
    break;
#endif

defaut:
    log_error("not support update msg:%x\n", msg);
    break;
    }

}

void testbox_update_init(void)
{
    //if (CONFIG_UPDATE_ENABLE && (CONFIG_UPDATE_BLE_TEST_EN || CONFIG_UPDATE_BT_LMP_EN))
    {
        /* printf("testbox msg handle reg:%x\n", testbox_update_msg_handle); */
        /* btctrler_testbox_update_msg_handle_register(testbox_update_msg_handle); */
    }
}

