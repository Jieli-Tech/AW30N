#include "rcsp_bluetooth.h"
#include "rcsp_user_update.h"
#include "rcsp_ch_loader_download.h"
#include "code_v2/update.h"
#include "third_party/rcsp/JL_rcsp_protocol.h"
#include "msg/msg.h"
#include "custom_cfg.h"

#if (UPDATE_V2_EN && RCSP_BTMATE_EN)

#define LOG_TAG_CONST       RCSP_MODE
//#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[APP-UPDATE]"
#include "log.h"

#define DEV_UPDATE_FILE_INFO_OFFEST  0x00//0x40
#define DEV_UPDATE_FILE_INFO_LEN     0x00//(0x10 + VER_INFO_EXT_COUNT * (VER_INFO_EXT_MAX_LEN + 1))

typedef enum {
    UPDATA_START = 0x00,
    UPDATA_REV_DATA,
    UPDATA_STOP,
} UPDATA_BIT_FLAG;

enum {
    UPDATE_FLAG_OK,
    UPDATE_FLAG_LOW_POWER,
    UPDATE_FLAG_FW_INFO_ERR,
    UPDATE_FLAG_FW_INFO_CONSISTENT,
};

typedef struct _update_mode_t {
    u8 opcode;
    u8 opcode_sn;
} update_mode_t;

extern const int support_dual_bank_update_en;
extern u8 check_le_pakcet_sent_finish_flag(void);

static u8 update_flag = 0;
static u8 rcsp_update_status;
static u16 ble_discon_timeout;
static void (*fw_update_block_handle)(u8 state, void *buf, int len) = NULL;
static update_mode_t update_record_info;

static void JL_rcsp_resp_dev_update_file_info_offest(u8 OpCode, u8 OpCode_SN)
{
    u8 data[4 + 2];
    u16 update_file_info_offset = DEV_UPDATE_FILE_INFO_OFFEST;
    u16 update_file_info_len = DEV_UPDATE_FILE_INFO_LEN;
    WRITE_BIG_U32(data + 0, update_file_info_offset);
    WRITE_BIG_U16(data + 4, update_file_info_len);
    JL_CMD_response_send(OpCode, JL_PRO_STATUS_SUCCESS, OpCode_SN, data, sizeof(data));
}

static void JL_resp_inquire_device_if_can_update(u8 OpCode, u8 OpCode_SN, u8 update_sta)
{
    u8 data[1];
    data[0] = update_sta;
    JL_CMD_response_send(OpCode, JL_PRO_STATUS_SUCCESS, OpCode_SN, data, sizeof(data));
}

static u32 rcsp_update_data_read(void *priv, u32 offset_addr, u16 len)
{
    u32 err = 0;
    u8 data[4 + 2];
    WRITE_BIG_U32(data, offset_addr);
    WRITE_BIG_U16(data + 4, len);
    err = JL_CMD_send(JL_OPCODE_SEND_FW_UPDATE_BLOCK, data, sizeof(data), JL_NEED_RESPOND);
    return err;
}

static void JL_controller_save_curr_cmd_para(u8 OpCode, u8 OpCode_SN)
{
    update_record_info.opcode = OpCode;
    update_record_info.opcode_sn = OpCode_SN;
}

static void JL_controller_get_curr_cmd_para(u8 *OpCode, u8 *OpCode_SN)
{
    *OpCode = update_record_info.opcode;
    *OpCode_SN = update_record_info.opcode_sn;
}

static u32 JL_controller_resp_get_dev_refresh_fw_status(u8 OpCode, u8 OpCode_SN, u8 result)
{
    u32 err = 0;
    u8 data[1];
    data[0] = result;
    err = JL_CMD_response_send(OpCode, JL_PRO_STATUS_SUCCESS, OpCode_SN, data, sizeof(data));
    return err;
}

static u32 rcsp_update_status_response(void *priv, u8 status)
{
    u8 OpCode;
    u8 OpCode_SN;
    u32 err = 0;
    JL_controller_get_curr_cmd_para(&OpCode, &OpCode_SN);
    if (JL_OPCODE_GET_DEVICE_REFRESH_FW_STATUS == OpCode) {
        err = JL_controller_resp_get_dev_refresh_fw_status(OpCode, OpCode_SN, status);
    }
    return err;
}

static void register_receive_fw_update_block_handle(void (*handle)(u8 state, void *buf, int len))
{
    fw_update_block_handle = handle;
}

static void rcsp_loader_download_result_handle(void *priv, u8 type, u8 cmd)
{
    /* if (UPDATE_LOADER_OK == cmd) {	 */
    if (1 == cmd) {
        set_jl_update_flag(1);
        if (support_dual_bank_update_en) {
            log_info(">>>rcsp update succ\n");
            update_result_set(UPDATA_SUCC);
        } else {
            log_info(">>>update loader err\n");
        }
    }
}

static bool check_ble_all_packet_sent(void)
{
    if (check_le_pakcet_sent_finish_flag()) {
        return TRUE;
    } else {
        return FALSE;
    }
}

extern u8 ble_update_ready_jump_flag;
static void ble_discon_timeout_handle(void *priv)
{
    ble_update_ready_jump_flag = 0;
    rcsp_msg_post(MSG_JL_UPDATE_START, 1, 0);
}

static void rcsp_update_private_param_fill(UPDATA_PARM *p)
{
    u32 exif_addr;

    exif_addr = ex_cfg_fill_content_api();
    update_param_priv_fill(p, (void *)&exif_addr, sizeof(exif_addr));
#if UPDATA_KEEP_IO_ENABLE
    update_keep_io(p);
#endif
}

static void rcsp_update_before_jump_handle(int type)
{
    log_info("latch reset\n");
    common_update_before_jump_reset_handle();
}

void JL_rcsp_update_cmd_resp(void *priv, u8 OpCode, u8 OpCode_SN, u8 *data, u16 len)
{
    u8 msg[4];
    log_info("%s\n", __func__);
    switch (OpCode) {
    case JL_OPCODE_GET_DEVICE_UPDATE_FILE_INFO_OFFSET:
        if (0 == len) {
            log_info("JL_OPCODE_GET_DEVICE_UPDATE_FILE_INFO_OFFSET\n");
            rcsp_msg_post(MSG_JL_GET_DEV_UPDATE_FILE_INFO_OFFSET, 2, OpCode, OpCode_SN);
        } else {
            log_info("JL_OPCODE_GET_DEVICE_UPDATE_FILE_INFO_OFFSET err\n");
        }
        break;
    case JL_OPCODE_INQUIRE_DEVICE_IF_CAN_UPDATE:
        log_info("JL_OPCODE_INQUIRE_DEVICE_IF_CAN_UPDATE\n");
        if (len) {
#if !TRANS_DATA_SPPLE_EN
            set_curr_update_type(data[0]);
#endif
            rcsp_msg_post(MSG_JL_INQUIRE_DEVEICE_IF_CAN_UPDATE, 3, OpCode, OpCode_SN, 0);
        }
        break;
    case JL_OPCODE_ENTER_UPDATE_MODE:
        log_info("JL_OPCODE_ENTER_UPDATE_MODE\n");
        rcsp_msg_post(MSG_JL_ENTER_UPDATE_MODE, 2, OpCode, OpCode_SN);
        break;
    case JL_OPCODE_GET_DEVICE_REFRESH_FW_STATUS:
        log_info("JL_OPCODE_GET_DEVICE_REFRESH_FW_STATUS\n");
        JL_controller_save_curr_cmd_para(OpCode, OpCode_SN);
        if (fw_update_block_handle) {
            fw_update_block_handle(UPDATA_STOP, NULL, 0);
        }
        break;
    case JL_OPCODE_SET_DEVICE_REBOOT:
        log_info("JL_OPCODE_SET_DEVICE_REBOOT\n");
        if (support_dual_bank_update_en) {
            system_reset(UPDATE_FLAG);
        }
        break;
    }
}

void JL_rcsp_msg_deal(RCSP_MSG msg, int argc, int *argv)
{
    u16 remote_file_version;
    u8 can_update_flag = UPDATE_FLAG_FW_INFO_ERR;
    switch (msg) {
    case RCSP_MSG_NORMAL_NORMAL_PROCESS:
        break;
    case MSG_JL_GET_DEV_UPDATE_FILE_INFO_OFFSET:
        log_info("MSG_JL_GET_DEV_UPDATE_FILE_INFO_OFFSET\n");
        rcsp_update_status = 1;
        if (argc < 2) {
            log_info("err: MSG_JL_GET_DEV_UPDATE_FILE_INFO_OFFSET too few argument, argc is %d\n", argc);
            return;
        }
        JL_rcsp_resp_dev_update_file_info_offest((u8)argv[0], (u8)argv[1]);
        break;
    case MSG_JL_INQUIRE_DEVEICE_IF_CAN_UPDATE:
        log_info("MSG_JL_INQUIRE_DEVEICE_IF_CAN_UPDATE\n");
        if (argc < 2) {
            log_info("err: MSG_JL_INQUIRE_DEVEICE_IF_CAN_UPDATE too few argument, argc is %d\n", argc);
        }
        can_update_flag = UPDATE_FLAG_OK;
        JL_resp_inquire_device_if_can_update((u8)argv[0], (u8)argv[1], can_update_flag);
        if (0 == support_dual_bank_update_en) {
            post_event(EVENT_OTA_UPDATE);
        }
        break;
    case MSG_JL_DEV_DISCONNECT:
        if (check_ble_all_packet_sent()) {
            log_info("MSG_JL_DEV_DISCONNECT\n");
            JL_ble_disconnect();
            ble_update_ready_jump_flag = 1;
#if RCSP_BLE_DEAL_OK
            if (check_edr_is_disconnct()) {
                log_info("-need discon edr\n");
                user_send_cmd_prepare(USER_CTRL_POWER_OFF, 0, NULL);
            }
#endif
            ble_discon_timeout = sys_timeout_add(NULL, ble_discon_timeout_handle, 1000);
        } else {
            log_info("W");
            rcsp_msg_post(MSG_JL_DEV_DISCONNECT, 1, 0);
        }
        break;
#if 0
    case MSG_JL_LOADER_DOWNLOAD_START:
        log_info("MSG_JL_LOADER_DOWNLOAD_START\n");
        rcsp_update_data_api_register(rcsp_update_data_read, rcsp_update_status_response);
        register_receive_fw_update_block_handle(rcsp_update_handle);
        if (RCSP_BLE == get_curr_device_type()) {
            rcsp_update_loader_download_init(BLE_APP_UPDATA, rcsp_loader_download_result_handle);
        } else if (RCSP_SPP == get_curr_device_type()) {
            rcsp_update_loader_download_init(SPP_APP_UPDATA, rcsp_loader_download_result_handle);
        }
        break;
#endif
    case MSG_JL_UPDATE_START:
#if RCSP_BLE_DEAL_OK
        if (check_edr_is_disconnct()) {
            rcsp_log_info("b");
            rcsp_msg_post(MSG_JL_UPDATE_START, 0);
            break;
        }
#endif

        rcsp_update_jump_to_loader_handle(NULL);
        break;
    case MSG_JL_LOADER_DOWNLOAD_START:
    case MSG_JL_BLE_UPDATE_START:
    case MSG_JL_ENTER_UPDATE_MODE:
    case MSG_JL_SPP_UPDATE_START:
        break;
    }
}

void rcsp_update_jump_to_loader_handle(void *priv)
{
    if (RCSP_BLE == get_curr_device_type()) {
        log_info("BLE_APP_UPDATE\n");
        update_mode_api_v2(BLE_APP_UPDATA,
                           rcsp_update_private_param_fill,
                           rcsp_update_before_jump_handle);
#if TRANS_DATA_SPPLE_EN
    } else {
        log_info("USB_HID_UPDATE\n");
        update_mode_api_v2(USB_HID_UPDATA,
                           rcsp_update_private_param_fill,
                           rcsp_update_before_jump_handle);
#endif
    }
}

void app_ota_update_handle(void)
{
    if (!CONFIG_UPDATE_APP_OTA_EN) {
        return;
    }

    rcsp_update_data_api_register(rcsp_update_data_read, rcsp_update_status_response);
    register_receive_fw_update_block_handle(rcsp_update_handle);
    if (RCSP_BLE == get_curr_device_type()) {
        rcsp_update_loader_download_init(BLE_APP_UPDATA, rcsp_loader_download_result_handle);
    } else if (RCSP_SPP == get_curr_device_type()) {
        rcsp_update_loader_download_init(SPP_APP_UPDATA, rcsp_loader_download_result_handle);
#if TRANS_DATA_SPPLE_EN
    } else {
        rcsp_update_loader_download_init(USB_HID_UPDATA, rcsp_loader_download_result_handle);
#endif
    }

}
void app_update_msg_handle(int msg)
{
    switch (msg) {
    case MSG_BLE_APP_UPDATE_START:
        rcsp_update_data_api_register(rcsp_update_data_read, rcsp_update_status_response);
        register_receive_fw_update_block_handle(rcsp_update_handle);
        if (RCSP_BLE == get_curr_device_type()) {
            rcsp_update_loader_download_init(BLE_APP_UPDATA, rcsp_loader_download_result_handle);
        } else if (RCSP_SPP == get_curr_device_type()) {
            rcsp_update_loader_download_init(SPP_APP_UPDATA, rcsp_loader_download_result_handle);
#if TRANS_DATA_SPPLE_EN
        } else {
            rcsp_update_loader_download_init(USB_HID_UPDATA, rcsp_loader_download_result_handle);
#endif
        }
        break;
    }
}

void JL_rcsp_update_cmd_receive_resp(void *priv, u8 OpCode, u8 status, u8 *data, u16 len)
{
    switch (OpCode) {
    case JL_OPCODE_SEND_FW_UPDATE_BLOCK:
        if (fw_update_block_handle) {
            fw_update_block_handle(UPDATA_REV_DATA, data, len);
        }
        break;
    default:
        break;
    }
}

u8 get_jl_update_flag(void)
{
    return update_flag;
}

void set_jl_update_flag(u8 flag)
{
    log_info("%s, %x\n", __func__, flag);
    update_flag = flag;
}

bool app_get_jl_updating_flag(void)
{
    if (classic_update_task_exist_flag_get() || get_jl_update_flag()) {
        // 升级中
        return true;
    }
    return false;
}

#endif

