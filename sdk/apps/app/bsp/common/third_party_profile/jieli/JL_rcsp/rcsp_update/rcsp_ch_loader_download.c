#include "rcsp_ch_loader_download.h"
#include "rcsp_user_update.h"
#include "code_v2/update_loader_download.h"
#include "code_v2/update.h"
#include "rcsp_bluetooth.h"
#include "tick_timer_driver.h"
#include "third_party/rcsp/JL_rcsp_protocol.h"
#include "app_modules.h"

#include "log.h"

#if (UPDATE_V2_EN && RCSP_BTMATE_EN)

typedef enum __DEVICE_REFRESH_FW_STATUS {
    DEVICE_UPDATE_STA_SUCCESS = 0,      //升级成功(default)
    DEVICE_UPDATE_STA_VERIFY_ERR,       //升级完校验代码出错(default)
    DEVICE_UPDATE_STA_FAIL,             //升级失败(default)
    DEVICE_UPDATE_STA_KEY_ERR,          //加密key不匹配
    DEVICE_UPDATE_STA_FILE_ERR,         //升级文件出错
    DEVICE_UPDATE_STA_TYPE_ERR,         //升级类型出错,仅code_type;
    //DEVICE_UPDATE_STA_MAX_ERR,
    DEVICE_UPDATE_STA_LOADER_DOWNLOAD_SUCC = 0x80,
} DEVICE_UPDATE_STA;

enum {
    BT_UPDATE_OVER = 0,
    BT_UPDATE_KEY_ERR,
    BT_UPDATE_CONNECT_ERR,
};

typedef enum {
    UPDATA_START = 0x00,
    UPDATA_REV_DATA,
    UPDATA_STOP,
} UPDATA_BIT_FLAG;

enum {
    BT_SEEK_SET = 0x01,
    BT_SEEK_CUR = 0x02,

    BT_SEEK_TYPE_UPDATE_LEN = 0x10,
};

enum {
    SEEK_SET = 0x0,
    SEEK_CUR = 0x1,
    SEEK_END = 0X2,
};

typedef struct _rcsp_update_param_t {
    u32 state;
    u32 read_len;
    u32 need_rx_len;
    u8 *read_buf;
    void (*resume_hdl)(void *priv);
    int (*sleep_hdl)(void *priv);
    u32(*data_send_hdl)(void *priv, u32 offset, u16 len);
    u32(*send_update_status_hdl)(void *priv, u8 state);
    u32 file_offset;
    u8 seek_type;
} rcsp_update_param_t;

static rcsp_update_param_t  rcsp_update_param;
#define __this (&rcsp_update_param)

#define RETRY_TIMES     3
#define UPDATE_TIMEOUT	50	// 50*10ms

void rcsp_update_data_api_register(u32(*data_send_hdl)(void *priv, u32 offset, u16 len), u32(*send_update_status_hdl)(void *priv, u8 state))
{
    __this->data_send_hdl = data_send_hdl;
    __this->send_update_status_hdl = send_update_status_hdl;
}

void rcsp_update_data_api_unregister(void)
{
    __this->data_send_hdl = NULL;
    __this->send_update_status_hdl = NULL;
}

void rcsp_update_handle(u8 state, void *buf, int len)
{
    /* deg_puts("R"); */
    if (state != __this->state) {
        printf(">>>rcsp state err\n");
        return;
    }

    switch (state) {
    case UPDATA_REV_DATA:
        if (__this->read_buf) {
            memcpy(__this->read_buf, buf, len);
            __this->read_len = len;
            __this->state = 0;
        }
        break;

    case UPDATA_STOP:
        __this->state = 0;
        break;
    }
}

static void rcsp_ch_update_init(void (*resume_hdl)(void *priv), int (*sleep_hdl)(void *priv))
{
    printf("------------rcsp_ch_update_init\n");

    /* rcsp_update_resume_hdl_register(resume_hdl, sleep_hdl); */
    //register_receive_fw_update_block_handle(rcsp_updata_handle);
    //}
}

static u8 update_result_handle(u8 err)
{
    u8 res = DEVICE_UPDATE_STA_LOADER_DOWNLOAD_SUCC;

    /* #if OTA_TWS_SAME_TIME_ENABLE */
    /*     tws_api_auto_role_switch_enable(); */
    /* #endif */

    if (err & UPDATE_RESULT_FLAG_BITMAP) {
        switch (err & 0x7f) {
        //升级文件错误
        case UPDATE_RESULT_FILE_SIZE_ERR:
        case UPDATE_RESULT_LOADER_SIZE_ERR:
        case UPDATE_RESULT_REMOTE_FILE_HEAD_ERR:
        case UPDATE_RESULT_LOCAL_FILE_HEAD_ERR:
        case UPDATE_RESULT_FILE_OPERATION_ERR:
        case UPDATE_RESULT_NOT_FIND_TARGET_FILE_ERR:
        case UPDATE_RESULT_PRODUCT_INFO_NOT_MATCH:
            res = DEVICE_UPDATE_STA_FILE_ERR;
            break;
        //文件内容校验失败
        case UPDATE_RESULT_LOADER_VERIFY_ERR:
        case UPDATE_RESULT_FLASH_DATA_VERIFY_ERR:
            res = DEVICE_UPDATE_STA_VERIFY_ERR;
            break;

        }
    } else if (BT_UPDATE_OVER == err || UPDATE_RESULT_BT_UPDATE_OVER == err) {
        if (support_dual_bank_update_en) {
            res = DEVICE_UPDATE_STA_SUCCESS;
        } else {
            res = DEVICE_UPDATE_STA_LOADER_DOWNLOAD_SUCC;
        }
    } else if (BT_UPDATE_KEY_ERR == err) {
        res = DEVICE_UPDATE_STA_KEY_ERR;
    } else {
        res = DEVICE_UPDATE_STA_FAIL;
    }

    return res;
}

static u16 rcsp_f_open(void)
{
    printf(">>>rcsp_f_open\n");
    __this->file_offset = 0;
    __this->seek_type = BT_SEEK_SET;
    return 1;
}

static u16 rcsp_f_read(void *fp, u8 *buff, u16 len)
{
    //printf("===rcsp_read:%x %x\n", __this->file_offset, len);
    u8 retry_cnt = 0;
    u32 timer = 0;
    __this->need_rx_len = len;
    __this->state = UPDATA_REV_DATA;
    __this->read_len = 0;
    __this->read_buf = buff;
#if (0 == TRANS_DATA_SPPLE_EN)
    if (!get_rcsp_connect_status()) {   //如果已经断开连接直接返回-1
        return -1;
    }
#endif
    __this->data_send_hdl(fp, __this->file_offset, len);

__RETRY:
    timer = jiffies + UPDATE_TIMEOUT;
    while (!((0 == __this->state) && (__this->read_len == len))) {
        wdt_clear();
#if (0 == TRANS_DATA_SPPLE_EN)
        if (0 == get_rcsp_connect_status()) {
            len = (u16) - 1;
            break;
        }
#endif
        if (timer < jiffies) { //500ms内如果
            if (retry_cnt++ > RETRY_TIMES) {	//循环3次后超时退出
                len = (u16) - 1;
                break;
            } else {//过了500ms重新获取500ms
                goto __RETRY;
            }
        }
    }

    if ((u16) - 1 != len) {
        __this->file_offset += len;
    }
    return len;
}

static u16 rcsp_f_stop(u8 err)
{
    u32 timer = 0;
    err = update_result_handle(err);
    __this->state = UPDATA_STOP;
    printf(">>>rcsp_stop:%x\n", __this->state);

    if (__this->data_send_hdl) {
        __this->data_send_hdl(NULL, 0, 0);
    }
    timer = jiffies + UPDATE_TIMEOUT;
    while (!(0 == __this->state)) {
#if TRANS_DATA_SPPLE_EN
        if (timer < jiffies) {
#else
        if (0 == get_rcsp_connect_status()) {
            break;
        }
        if (timer < jiffies) {
#endif
            break;
        }
    }
    if (__this->send_update_status_hdl) {
        __this->send_update_status_hdl(NULL, err);
    }

    return 1;
}

static int rcsp_f_seek(void *fp, u8 type, u32 offset)
{
    if (type == SEEK_SET) {
        __this->file_offset = offset;
        __this->seek_type = BT_SEEK_SET;
    } else if (type == SEEK_CUR) {
        __this->file_offset += offset;
        __this->seek_type = BT_SEEK_CUR;
    }

    /* lib_printf("---------UPDATA_seek type %d, offsize %d----------\n", bt_seek_type, bt_file_offset); */
    return 0;//FR_OK;
}

static int rcsp_notify_update_content_size(void *priv, u32 size)
{
    int err = 0;

    u8 data[4];

    WRITE_BIG_U32(data, size);

    printf("send content_size:%x\n", size);
    err = JL_CMD_send(JL_OPCODE_NOTIFY_UPDATE_CONENT_SIZE, data, sizeof(data), JL_NEED_RESPOND);

    return err;
}

const update_op_api_t rcsp_update_op = {
    .ch_init = rcsp_ch_update_init,
    .f_open = rcsp_f_open,
    .f_read = rcsp_f_read,
    .f_seek = rcsp_f_seek,
    .f_stop = rcsp_f_stop,
    .notify_update_content_size = rcsp_notify_update_content_size,
};

static void rcsp_update_state_cbk(int type, u32 state, void *priv)
{
    update_ret_code_t *ret_code = (update_ret_code_t *)priv;
    if (ret_code) {
        printf("state:%x err:%x\n", ret_code->stu, ret_code->err_code);
    }
    switch (state) {
    case UPDATE_CH_EXIT:
        if (UPDATE_DUAL_BANK_IS_SUPPORT()) {
            if ((0 == ret_code->stu) && (0 == ret_code->err_code)) {
                set_jl_update_flag(1);
                printf(">>>rcsp update succ\n");
                update_result_set(UPDATA_SUCC);

            } else {
                update_result_set(UPDATA_DEV_ERR);
                printf(">>>rcsp update succ\n");
            }
        } else {
            if ((0 == ret_code->stu) && (0 == ret_code->err_code || UPDATE_RESULT_BT_UPDATE_OVER == ret_code->err_code)) {
                set_jl_update_flag(1);

            } else {
                // 主动断开ble
                // cpu_reset
                printf("update fail, cpu reset!!!\n");
                system_reset(UPDATE_FLAG);
            }
        }
        break;
    }
}

void rcsp_update_loader_download_init(int update_type, void (*result_cbk)(void *priv, u8 type, u8 cmd))
{
    update_mode_info_t info = {
        .type = update_type,
        .state_cbk = rcsp_update_state_cbk,
        .p_op_api = &rcsp_update_op,
        .task_en = 1,
    };
    app_active_update_task_init(&info);
}

#endif

