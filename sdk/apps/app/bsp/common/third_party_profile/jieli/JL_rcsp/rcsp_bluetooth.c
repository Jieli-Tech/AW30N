#include "rcsp_bluetooth.h"
#include "rcsp_user_update.h"
#include "rcsp_ch_loader_download.h"
#include "rcsp_msg.h"
#include "avctp_user.h"
#include "msg.h"
#include "third_party/rcsp/JL_rcsp_protocol.h"
#include "third_party/rcsp/attr.h"
#include "third_party/rcsp/JL_rcsp_api.h"
#include "third_party/rcsp/rcsp_auth.h"
#include "third_party/rcsp/rcsp_auth.h"
#include "custom_cfg.h"
#include "rcsp_hid_inter.h"

#include "log.h"
#if RCSP_BTMATE_EN

#define ATTR_TYPE_PROTOCOL_VERSION	0
#define ATTR_TYPE_SYS_INFO			1
#define ATTR_TYPE_EDR_ADDR			2
#define	ATTR_TYPE_PLATFORM			3
#define	ATTR_TYPE_FUNCTION_INFO		4
#define	ATTR_TYPE_DEV_VERSION		5
#define	ATTR_TYPE_SDK_TYPE			6
#define	ATTR_TYPE_UBOOT_VERSION		7
#define	ATTR_TYPE_DOUBLE_PARITION	8
#define	ATTR_TYPE_UPDATE_STATUS		9
#define	ATTR_TYPE_DEV_VID_PID		10
#define	ATTR_TYPE_DEV_AUTHKEY		11
#define	ATTR_TYPE_DEV_PROCODE		12
#define	ATTR_TYPE_DEV_MAX_MTU		13
#define	ATTR_TYPE_MD5_GAME_SUPPORT	19

#define RCSP_USE_BLE      0
#define RCSP_USE_SPP      1
#define RCSP_CHANNEL_SEL  RCSP_USE_BLE

#pragma pack(1)

struct _SYS_info {
    u8 bat_lev;
    u8 sys_vol;
    u8 max_vol;
    u8 reserve;
};

struct _EDR_info {
    u8 addr_buf[6];
    u8 profile;
    u8 state;
};

struct _DEV_info {
    u8 status;
    u32 usb_handle;
    u32 sd0_handle;
    u32 sd1_handle;
    u32 flash_handle;
};

struct _EQ_INFO {
    u8 mode;
    s8 gain_val[10];
};

struct _MUSIC_STATUS_info {
    u8 status;
    u32 cur_time;
    u32 total_time;
    u8 cur_dev;
};

struct _dev_version {
    u16 _sw_ver2: 4; //software l version
    u16 _sw_ver1: 4; //software m version
    u16 _sw_ver0: 4; //software h version
    u16 _hw_ver:  4; //hardware version
};

#pragma pack()

extern void ble_get_server_operation_table(struct ble_server_operation_t **interface_pt);

extern const int support_dual_bank_update_en;

struct JL_AI_VAR {
    ble_state_e JL_ble_status;
    struct ble_server_operation_t *rcsp_ble;
#if TRANS_DATA_SPPLE_EN
    struct rcsp_hid_operation_t *rcsp_hid;
#endif
    u32 feature_mask;
    u16 rcsp_timer_hdl;
    u8 device_type;
    u8 phone_platform;
    volatile u8 rcsp_run_flag;
    volatile u8 loop_resume_cnt;
    u8 new_reconn_flag;
};

struct JL_AI_VAR jl_ai_var = {
    .rcsp_run_flag = 0,
};

#define __this	(&jl_ai_var)
static u8 rcsp_buffer[0xA80] __attribute__((aligned(4)));

static const u8 link_key_data[16] = {0x06, 0x77, 0x5f, 0x87, 0x91, 0x8d, 0xd4, 0x23, 0x00, 0x5d, 0xf1, 0xd8, 0xcf, 0x0c, 0x14, 0x2b};

/* IRQ_FUNCTION_PROTOTYPE(IRQ_SOFT4_IDX, JL_rcsp_loop) */
__attribute__((interrupt("")))
static void JL_rcsp_loop(void)
{
    int msg[6], err;
    bit_clr_swi(4);
    // 获取cbuf中命令个数，顺序执行msg进行消息分发
    err = rcsp_get_msg(6, msg);
    if (MSG_NO_ERROR != err) {
        msg[0] = NO_MSG;
        printf("get msg err 0x%x\n", err);
    }
    switch (msg[0]) {
    case RCSP_MSG_NORMAL_NORMAL_PROCESS:
        // 收发
        JL_protocol_process();
        break;
    default:
        JL_rcsp_event_handler(msg[0], 5, msg + 1);
        break;
    }

    local_irq_disable();
    if (__this->loop_resume_cnt) {
        __this->loop_resume_cnt--;
        bit_set_swi(4);
    }
    local_irq_enable();
}

#if UPDATE_V2_EN
static volatile u8 JL_bt_chl = 0;
u8 JL_get_cur_bt_channel_sel(void);
#endif

static void rcsp_process_timer()
{
    rcsp_msg_post(RCSP_MSG_NORMAL_NORMAL_PROCESS, 1, 0);
}

static void JL_rcsp_resend_timer_opt(u8 flag, u32 usec)
{
    if (flag) {
        if (0 == __this->rcsp_timer_hdl) {
            __this->rcsp_timer_hdl = sys_timer_add(NULL, rcsp_process_timer, usec);
        }
    } else {
        if (__this->rcsp_timer_hdl) {
            sys_timer_del(__this->rcsp_timer_hdl);
            __this->rcsp_timer_hdl = NULL;
        }
    }
}

#define RCSP_TMP_BUF_LEN	(256)
static u8 g_rcsp_tmp_buf[RCSP_TMP_BUF_LEN];
static u32 JL_opcode_get_target_info(void *priv, u8 OpCode, u8 OpCode_SN, u8 *data, u16 len)
{
    u32 ret = 0;
    u32 mask = 0;
    u8 *buf = g_rcsp_tmp_buf;
    u8 offset = 0;
    __this->phone_platform = data[4];
    if (__this->phone_platform == ANDROID) {
        printf("phone_platform == ANDROID\n");
    } else if (__this->phone_platform == APPLE_IOS) {
        printf("phone_platform == APPLE_IOS\n");
    } else {
        printf("phone_platform ERR\n");
    }

    mask = READ_BIG_U32(data);
    printf("FEATURE MASK : %x\n", mask);

    if (mask & BIT(ATTR_TYPE_PROTOCOL_VERSION)) {
        printf("ATTR_TYPE_PROTOCOL_VERSION\n");
        u8 ver = get_rcsp_version();
        offset += add_one_attr(buf, RCSP_TMP_BUF_LEN, offset, ATTR_TYPE_PROTOCOL_VERSION, &ver, 1);
    }

    if (mask & BIT(ATTR_TYPE_SYS_INFO)) {
        printf("ATTR_TYPE_SYS_INFO\n");
        struct _SYS_info sys_info;
        sys_info.bat_lev = 0;
        sys_info.sys_vol = 0;
        sys_info.max_vol = 0;
        offset += add_one_attr(buf, RCSP_TMP_BUF_LEN, offset, ATTR_TYPE_SYS_INFO, \
                               (u8 *)&sys_info, sizeof(sys_info));
    }

    if (mask & BIT(ATTR_TYPE_EDR_ADDR)) {
        printf("ATTR_TYPE_EDR_ADDR\n");
        struct _EDR_info edr_info;
        extern const u8 *bt_get_mac_addr();
        u8 taddr_buf[6];
        memcpy(taddr_buf, bt_get_mac_addr(), 6);
        edr_info.addr_buf[0] =  taddr_buf[5];
        edr_info.addr_buf[1] =  taddr_buf[4];
        edr_info.addr_buf[2] =  taddr_buf[3];
        edr_info.addr_buf[3] =  taddr_buf[2];
        edr_info.addr_buf[4] =  taddr_buf[1];
        edr_info.addr_buf[5] =  taddr_buf[0];
        edr_info.profile = 0x0E;
#if (RCSP_CHANNEL_SEL == RCSP_USE_BLE)
        edr_info.profile &= ~BIT(7);
#else
        edr_info.profile |= BIT(7);
#endif
        /* extern u8 get_bt_connect_status(void); */
        /* if (get_bt_connect_status() ==  BT_STATUS_WAITINT_CONN) { */
        /*     edr_info.state = 0; */
        /* } else { */
        /*     edr_info.state = 1; */
        /* } */
        offset += add_one_attr(buf, RCSP_TMP_BUF_LEN, offset, ATTR_TYPE_EDR_ADDR, (u8 *)&edr_info, sizeof(struct _EDR_info));
    }

    if (mask & BIT(ATTR_TYPE_FUNCTION_INFO)) {
        printf("ATTR_TYPE_FUNCTION_INFO\n");
    }

    if (mask & BIT(ATTR_TYPE_DEV_VERSION)) {
        u8 tmp_ver = 0;
        offset += add_one_attr(buf, RCSP_TMP_BUF_LEN, offset, ATTR_TYPE_DEV_VERSION, (u8 *)&tmp_ver, 1);
    }

    if (mask & BIT(ATTR_TYPE_DOUBLE_PARITION)) {
        u8 double_partition_value;
        u8 ota_loader_need_download_flag;
        if (support_dual_bank_update_en) {
            double_partition_value = 0x1;
            ota_loader_need_download_flag = 0x00;
        } else {
            double_partition_value = 0x0;
            ota_loader_need_download_flag = 0x01;
        }
        u8 update_param[2] = {
            double_partition_value,
            ota_loader_need_download_flag,
        };
        offset += add_one_attr(buf, RCSP_TMP_BUF_LEN, offset, ATTR_TYPE_DOUBLE_PARITION, (u8 *)update_param, sizeof(update_param));
    }

    if (mask & BIT(ATTR_TYPE_UPDATE_STATUS)) {
        u8 update_status_value = 0x0;
        offset += add_one_attr(buf, RCSP_TMP_BUF_LEN, offset, ATTR_TYPE_UPDATE_STATUS, (u8 *)&update_status_value, sizeof(update_status_value));
    }

    if (mask & BIT(ATTR_TYPE_DEV_VID_PID)) {
        u8 temp_dev_vid_pid = 0;
        offset += add_one_attr(buf, RCSP_TMP_BUF_LEN, offset, ATTR_TYPE_DEV_VID_PID, (u8 *)&temp_dev_vid_pid, sizeof(temp_dev_vid_pid));
    }

    if (mask & BIT(ATTR_TYPE_SDK_TYPE)) {
        u8 sdk_type = 1;
        offset += add_one_attr(buf, RCSP_TMP_BUF_LEN, offset, ATTR_TYPE_SDK_TYPE, &sdk_type, 1);
    }

    if (mask & BIT(ATTR_TYPE_DEV_MAX_MTU)) {
        u16 rx_max_mtu = JL_packet_get_rx_max_mtu();
        u16 tx_max_mtu = JL_packet_get_tx_max_mtu();
        u8 t_buf[4];
        t_buf[0] = (tx_max_mtu >> 8) & 0xFF;
        t_buf[1] = tx_max_mtu & 0xFF;
        t_buf[2] = (rx_max_mtu >> 8) & 0xFF;
        t_buf[3] = rx_max_mtu & 0xFF;
        offset += add_one_attr(buf, RCSP_TMP_BUF_LEN, offset,  ATTR_TYPE_DEV_MAX_MTU, t_buf, 4);
    }

    if (mask & BIT(ATTR_TYPE_MD5_GAME_SUPPORT)) {
        printf("ATTR_TYPE_MD5_GAME_SUPPORT\n");
    }

__end:

    if (ret) {
        ret = JL_CMD_response_send(OpCode, JL_PRO_STATUS_FAIL, OpCode_SN, NULL, 0);
    } else {
        ret = JL_CMD_response_send(OpCode, JL_PRO_STATUS_SUCCESS, OpCode_SN, buf, offset);
    }

    return ret;
}

static void JL_rcsp_cmd_resp(void *priv, u8 OpCode, u8 OpCode_SN, u8 *data, u16 len)
{
    printf("JL_rcsp_cmd_resp op = %d\n", OpCode);
    switch (OpCode) {
    case JL_OPCODE_GET_TARGET_FEATURE:
        printf("JL_OPCODE_GET_TARGET_FEATURE\n");
        JL_rcsp_resend_timer_opt(1, 500);
        JL_opcode_get_target_info(priv, OpCode, OpCode_SN, data, len);
        break;
    case JL_OPCODE_SWITCH_DEVICE:
        __this->device_type = data[0];
        if (len > 1) {
            __this->new_reconn_flag	= data[1];
            JL_CMD_response_send(OpCode, JL_PRO_STATUS_SUCCESS, OpCode_SN, &__this->new_reconn_flag, 1);
        } else {
            JL_CMD_response_send(OpCode, JL_PRO_STATUS_SUCCESS, OpCode_SN, NULL, 0);
        }
#if UPDATE_V2_EN
        if (get_jl_update_flag()) {
            if (RCSP_BLE == JL_get_cur_bt_channel_sel()) {
                printf("BLE_CON START DISCON\n");
                rcsp_msg_post(MSG_JL_DEV_DISCONNECT, 1, 0);
            } else if (RCSP_SPP == JL_get_cur_bt_channel_sel()) {
                printf("WAIT_FOR_SPP_DISCON\n");
#if TRANS_DATA_SPPLE_EN
            } else {
                printf("RCSP HID DISCON\n");
                set_curr_update_type(RCSP_HID);
                sys_timeout_add(NULL, rcsp_update_jump_to_loader_handle, 200);
#endif
            }
        }
#endif
        break;
    default:
#if UPDATE_V2_EN
        if ((OpCode >= JL_OPCODE_GET_DEVICE_UPDATE_FILE_INFO_OFFSET) && \
            (OpCode <= JL_OPCODE_SET_DEVICE_REBOOT)) {
            JL_rcsp_update_cmd_resp(priv, OpCode, OpCode_SN, data, len);
        } else
#endif
        {
            JL_CMD_response_send(OpCode, JL_ERR_NONE, OpCode_SN, data, len);
        }
        break;
    }
}

static bool JL_ble_fw_ready(void *priv)
{
    return ((__this->JL_ble_status == BLE_ST_NOTIFY_IDICATE) ? true : false);
}

static s32 JL_ble_send(void *priv, void *data, u16 len)
{
    if ((__this->rcsp_ble != NULL) && (__this->JL_ble_status == BLE_ST_NOTIFY_IDICATE)) {
        int err = __this->rcsp_ble->send_data(NULL, (u8 *)data, len);
        /* rcsp_printf("send :%d\n", len); */
        if (len < 128) {
            /* rcsp_printf_buf(data, len); */
        } else {
            /* rcsp_printf_buf(data, 128); */
        }

        if (err == 0) {
            return 0;
        } else if (err == APP_BLE_BUFF_FULL) {
            return 1;
        }
    } else {
        printf("send err -1 !!\n");
    }

    return -1;
}

static u8 JL_rcsp_wait_resp_timeout(void *priv, u8 OpCode, u8 counter)
{
    printf("JL_rcsp_wait_resp_timeout\n");
    return 0;
}

static void JL_rcsp_cmd_recieve_resp(void *priv, u8 OpCode, u8 status, u8 *data, u16 len)
{
    printf("rec resp:%x\n", OpCode);
    switch (OpCode) {
    default:
#if UPDATE_V2_EN
        if ((OpCode >= JL_OPCODE_GET_DEVICE_UPDATE_FILE_INFO_OFFSET) && \
            (OpCode <= JL_OPCODE_SET_DEVICE_REBOOT)) {
            JL_rcsp_update_cmd_receive_resp(priv, OpCode, status, data, len);
        }
#endif
        break;
    }
}

// =================== RCSP BLE ====================== //
static const JL_PRO_CB JL_pro_BLE_callback = {
    .priv              = NULL,
    .fw_ready          = JL_ble_fw_ready,
    .fw_send           = JL_ble_send,
    .CMD_resp          = JL_rcsp_cmd_resp,
    /* .CMD_no_resp       = JL_rcsp_cmd_no_resp,     */
    .CMD_no_resp       = NULL,
    /* .DATA_resp         = JL_rcsp_data_resp,       */
    .DATA_resp         = NULL,
    /* .DATA_no_resp      = JL_rcsp_data_no_resp,    */
    .DATA_no_resp      = NULL,
    .CMD_recieve_resp  = JL_rcsp_cmd_recieve_resp,
    /* .DATA_recieve_resp = JL_rcsp_data_recieve_resp, */
    .DATA_recieve_resp = NULL,
    .wait_resp_timeout = JL_rcsp_wait_resp_timeout,
};

static int rcsp_ble_data_send(void *priv, u8 *buf, u16 len)
{
    printf("### rcsp_ble_data_send %d\n", len);
    int err = 0;
    if (__this->rcsp_ble != NULL) {
        err = __this->rcsp_ble->send_data(NULL, (u8 *)buf, len);
    }
    return err;

}

static void rcsp_ble_callback_set(void (*resume)(void), void (*recieve)(void *, void *, u16), void (*status)(void *, ble_state_e))
{
    if (__this->rcsp_ble) {
        printf("----0x%x   0x%x   0x%x  0x%x\n", __this->rcsp_ble, __this->rcsp_ble->regist_wakeup_send, __this->rcsp_ble->regist_recieve_cbk, __this->rcsp_ble->regist_state_cbk);
        __this->rcsp_ble->regist_wakeup_send(NULL, resume);
        __this->rcsp_ble->regist_recieve_cbk(NULL, recieve);
        __this->rcsp_ble->regist_state_cbk(NULL, status);
    }
}

static void JL_rcsp_control_exit(void)
{
    JL_protocol_exit();
}

static void JL_ble_status_callback(void *priv, u8 status)
{
    printf("JL_ble_status_callback==================== %d\n", status);
    __this->JL_ble_status = status;
    switch (status) {
    case BLE_ST_IDLE:
        /* JL_rcsp_control_exit(); */
        break;
    case BLE_ST_ADV:
        break;
    case BLE_ST_CONNECT:
        break;
    case BLE_ST_SEND_DISCONN:
        JL_rcsp_resend_timer_opt(0, 0);
        break;
    case BLE_ST_DISCONN:
        rcsp_exit();
#if UPDATE_V2_EN
        if (get_jl_update_flag()) {
            if (__this->rcsp_ble->adv_enable) {
                __this->rcsp_ble->adv_enable(NULL, 0);
            }
        }
#endif
        break;
    case BLE_ST_NOTIFY_IDICATE:
#if (0 == BT_CONNECTION_VERIFY)
        JL_rcsp_auth_reset();
#endif
        break;
    default:
        break;
    }
}

#if TRANS_DATA_SPPLE_EN
// =================== RCSP HID ====================== //
static void JL_rcsp_hid_status_callback(u8 status)
{

}

static int JL_rcsp_hid_data_send(void *priv, u8 *data, u16 len)
{
    printf("### rcsp_hid_data_send %d\n", len);
    int err = 0;
    if (__this->rcsp_hid != NULL) {
        err = __this->rcsp_hid->send_data(NULL, data, len);
    }
    return err;
}

static s32 JL_rcsp_hid_send(void *priv, void *buf, u16 len)
{
    if (len < 128) {
        printf("send: \n");
        put_buf(buf, (u32)len);
    }
    if ((__this->rcsp_hid != NULL) && (JL_rcsp_hid_fw_ready(NULL))) {
        return __this->rcsp_hid->send_data(NULL, buf, len);
    } else {
        printf("send err -1 !!\n");
    }
    return -1;
}

static const JL_PRO_CB JL_pro_HID_callback = {
    .priv              = NULL,
    .fw_ready          = JL_rcsp_hid_fw_ready,
    .fw_send           = JL_rcsp_hid_send,
    .CMD_resp          = JL_rcsp_cmd_resp,
    /* .DATA_resp         = JL_rcsp_data_resp, */
    .DATA_resp         = NULL,
    /* .CMD_no_resp       = JL_rcsp_cmd_no_resp, */
    .CMD_no_resp       = NULL,
    /* .DATA_no_resp      = JL_rcsp_data_no_resp, */
    .DATA_no_resp      = NULL,
    .CMD_recieve_resp  = JL_rcsp_cmd_recieve_resp,
    /* .DATA_recieve_resp = JL_rcsp_data_recieve_resp, */
    .DATA_recieve_resp = NULL,
    .wait_resp_timeout = JL_rcsp_wait_resp_timeout,
};

static void rcsp_hid_callback_set(void (*recieve)(void *, void *, u16), void (*status)(u8))
{
    printf("----0x%x   0x%x   0x%x\n", __this->rcsp_hid, __this->rcsp_hid->regist_recieve_cbk, __this->rcsp_hid->regist_state_cbk);
    __this->rcsp_hid->regist_recieve_cbk(NULL, recieve);
    __this->rcsp_hid->regist_state_cbk(NULL, status);
}

// =================================================== //
#endif

static void JL_rcsp_recieve_handle(void *priv, void *buf, u16 len)
{
    //put_buf(buf, len > 0x20 ? 0x20 : len);
    if (0 == BT_CONNECTION_VERIFY) {
        if (!JL_rcsp_get_auth_flag()) {
            JL_rcsp_auth_recieve(buf, len);
            return;
        }
    }

    JL_protocol_data_recieve(priv, buf, len);
}

void rcsp_dev_select(u8 type)
{
    switch (type) {
    case RCSP_BLE:
        printf("------RCSP_BLE-----\n");
#if UPDATE_V2_EN
        JL_bt_chl = RCSP_BLE;
#endif
        rcsp_ble_callback_set(JL_protocol_resume, JL_rcsp_recieve_handle, JL_ble_status_callback);
        JL_protocol_dev_switch(&JL_pro_BLE_callback);
        JL_rcsp_auth_init(rcsp_ble_data_send, (u8 *)link_key_data, NULL);
        break;
#if TRANS_DATA_SPPLE_EN
    case RCSP_HID:
#if UPDATE_V2_EN
        JL_bt_chl = RCSP_HID;
        set_curr_update_type(RCSP_HID);
#endif
        printf("------RCSP_HID-----\n");
        rcsp_ble_callback_set(NULL, NULL, NULL);
        rcsp_hid_callback_set(JL_rcsp_recieve_handle, JL_rcsp_hid_status_callback);
        JL_protocol_dev_switch(&JL_pro_HID_callback);
        JL_rcsp_auth_init(JL_rcsp_hid_data_send, NULL, NULL);
        break;
#endif
    case RCSP_SPP:
        break;
    }
}

static u8 g_rcsp_auth_buffer[16 * 17] AT(.rcsp_buf);
void rcsp_init()
{
    if (__this->rcsp_run_flag)	 {
        return;
    }

    memset((u8 *)__this, 0, sizeof(struct JL_AI_VAR));
    /* u32 size = rcsp_protocol_need_buf_size(); */
    /* rcsp_printf("rcsp need buf size:%x\n", size); */
    /* rcsp_buffer = zalloc(size); */
    /* ASSERT(rcsp_buffer, "no, memory for rcsp_init\n"); */
    /* JL_protocol_init(rcsp_buffer, size); */
    printf("rcsp need buf size:%x\n", rcsp_protocol_need_buf_size());
    JL_protocol_init(rcsp_buffer, sizeof(rcsp_buffer));
    rcsp_auth_init(&g_rcsp_auth_buffer, sizeof(g_rcsp_auth_buffer));
    request_irq(IRQ_SOFT4_IDX, 1, JL_rcsp_loop, 0);
    ble_get_server_operation_table(&__this->rcsp_ble);
#if TRANS_DATA_SPPLE_EN
    rcsp_hid_get_operation_table(&__this->rcsp_hid);
    rcsp_dev_select(RCSP_HID);
#else
    rcsp_dev_select(RCSP_BLE);
#endif
    __this->rcsp_run_flag = 1;
    rcsp_msg_init();
#if (0 != BT_CONNECTION_VERIFY)
    JL_rcsp_set_auth_flag(1);
#endif
}

void rcsp_exit(void)
{
    printf("####  rcsp_exit_cb\n");
    rcsp_update_data_api_unregister();
    // 如果进入了某一模式，可以这里退出
    __this->rcsp_run_flag = 0;
}

void rcsp_resume_do(void)
{
    local_irq_disable();
    // 命令入cbuf
    if (__this->loop_resume_cnt < 0xff) {
        __this->loop_resume_cnt++;
    }
    local_irq_enable();
    bit_set_swi(4);
}

void JL_rcsp_resume_do(void)
{
    // 普通rcsp流程事件
    rcsp_msg_post(RCSP_MSG_NORMAL_NORMAL_PROCESS, 1, 0);
}

u8 get_rcsp_connect_status(void)
{
#if UPDATE_V2_EN
    if (RCSP_BLE == JL_get_cur_bt_channel_sel()) {
        if (__this->JL_ble_status == BLE_ST_CONNECT || __this->JL_ble_status == BLE_ST_NOTIFY_IDICATE) {
            return 1;
        } else {
            return 0;
        }
        /* } else { */
        /*     return __this->JL_spp_status; */
    }
#endif
    return 0;
}

#if UPDATE_V2_EN
u8 JL_get_cur_bt_channel_sel(void)
{
    return JL_bt_chl;
}

void JL_ble_disconnect(void)
{
    __this->rcsp_ble->disconnect(NULL);
}

u8 get_curr_device_type(void)
{
    return __this->device_type;
}

void set_curr_update_type(u8 type)
{
    __this->device_type = type;
}

u8 get_rcsp_support_new_reconn_flag(void)
{
    return __this->new_reconn_flag;
}

void app_update_start(int msg)
{
    app_update_msg_handle(msg);
}
#endif

#endif

