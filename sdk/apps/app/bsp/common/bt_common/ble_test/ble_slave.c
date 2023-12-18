#include "bt_ble.h"
#include "test_app_config.h"
#include "clock.h"
#include "rcsp_bluetooth.h"

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[ble_s]"
#include "log.h"

#if (TESTE_BLE_EN)
#if (SLAVE)
#define TEST_TIMER_SEND_DATA_RATE    0  //timer测试上行发送数据
#define TEST_SEND_DATA_RATE          0  //测试上行发送数据
#define TEST_SEND_HANDLE_VAL         ATT_CHARACTERISTIC_ae02_01_VALUE_HANDLE
/* #define TEST_SEND_HANDLE_VAL         ATT_CHARACTERISTIC_ae05_01_VALUE_HANDLE */
#define EXT_ADV_MODE_EN              0

#define TEST_AUDIO_DATA_UPLOAD       0 //测试文件上传


#if 0
#define log_info(x, ...)  printf("[LE_TRANS]" x " ", ## __VA_ARGS__)
#define log_info_hexdump(...)
#define puts(...)
#else
#define log_info(...)
#define log_info_hexdump(...)
#define puts(...)
#endif


//ATT发送的包长,    note: 20 <=need >= MTU
#define ATT_LOCAL_MTU_SIZE        (300)/*(251)*/
//ATT缓存的buffer大小,  note: need >= 20,可修改
#define ATT_SEND_CBUF_SIZE        (1024)

//共配置的RAM
#define ATT_RAM_BUFSIZE           (ATT_CTRL_BLOCK_SIZE + ATT_LOCAL_MTU_SIZE + ATT_SEND_CBUF_SIZE)                   //note:
static u8 att_ram_buffer[ATT_RAM_BUFSIZE] __attribute__((aligned(4)));

#if TEST_SEND_DATA_RATE
static u32 test_data_count, err_send_data_count = 0;
static u32 server_timer_handle = 0;
static u8 test_data_start;
#elif TEST_TIMER_SEND_DATA_RATE
static u8 send_buffer_test[10];
static u32 err_send_data_count = 0;
static u8 test_data_start;
#endif

/*
 打开流控使能后,确定使能接口 att_server_flow_enable 被调用
 然后使用过程 通过接口 att_server_flow_hold 来控制流控开关
 注意:流控只能控制对方使用带响应READ/WRITE等命令方式
 例如:ATT_WRITE_REQUEST = 0x12
 */
#define ATT_DATA_RECIEVT_FLOW           0//流控功能使能

//---------------
// 广播周期 (unit:0.625ms)
#define ADV_INTERVAL_MIN          (160)

#define HOLD_LATENCY_CNT_MIN  (3)  //(0~0xffff)
#define HOLD_LATENCY_CNT_MAX  (20) //(0~0xffff)
#define HOLD_LATENCY_CNT_ALL  (0xffff)

static volatile hci_con_handle_t con_handle;

//加密设置
/* static const uint8_t sm_min_key_size = 7; */

//连接参数更新请求设置
//是否使能参数请求更新,0--disable, 1--enable
static const uint8_t connection_update_enable = 1; ///0--disable, 1--enable
//当前请求的参数表index
static uint8_t connection_update_cnt = 0; //

//参数表
static struct conn_update_param_t slv_conn_param;
static const struct conn_update_param_t connection_param_table[] = {
    /* {2, 2, 0, 600},//11 */
    {8, 8, 0, 400},//3.7
    {8,  20, 10, 600},
    {12, 28, 4, 600},//3.7
    {12, 24, 30, 600},//3.05
};
#define SLV_UPDATE_WAITING  BIT(0)
static volatile u8 conn_parm_update_flag;

//共可用的参数组数
#define CONN_PARAM_TABLE_CNT      (sizeof(connection_param_table)/sizeof(struct conn_update_param_t))

#if (ATT_RAM_BUFSIZE < 64)
#error "adv_data & rsp_data buffer error!!!!!!!!!!!!"
#endif

//用户可配对的，这是样机跟客户开发的app配对的秘钥
/* const u8 link_key_data[16] = {0x06, 0x77, 0x5f, 0x87, 0x91, 0x8d, 0xd4, 0x23, 0x00, 0x5d, 0xf1, 0xd8, 0xcf, 0x0c, 0x14, 0x2b}; */
#define EIR_TAG_STRING   0xd6, 0x05, 0x08, 0x00, 'J', 'L', 'A', 'I', 'S', 'D','K'
static const char user_tag_string[] = {EIR_TAG_STRING};

static u8 adv_data_len;
static u8 adv_data[ADV_RSP_PACKET_MAX];//max is 31
static u8 scan_rsp_data_len;
static u8 scan_rsp_data[ADV_RSP_PACKET_MAX];//max is 31

/* #define adv_data       &att_ram_buffer[0] */
/* #define scan_rsp_data  &att_ram_buffer[32] */

static char gap_device_name[BT_NAME_LEN_MAX] = "jl_kkk_test";
static u8 gap_device_name_len = 0; //名字长度，不包含结束符
static u8 ble_work_state = 0;      //ble 状态变化
static u8 adv_ctrl_en;             //广播控制

static u8 test_read_write_buf[4];

static int (*update_recieve_callback)(void *priv, u8 *buf, u16 len) = NULL;
static int (*app_recieve_callback)(u8 *buf, u16 len) = NULL;
static void (*app_ble_state_callback)(void *priv, ble_state_e state) = NULL;
static void (*ble_resume_send_wakeup)(void) = NULL;
static u32 channel_priv;

#if TEST_TIMER_SEND_DATA_RATE
static void timer_init(u8 timer_ch, u32 ms);
#endif
static int app_send_user_data_check(u16 len);
static int app_send_user_data_do(void *priv, u8 *data, u16 len);
static int app_send_user_data(u16 handle, u8 *data, u16 len, u8 handle_type);
static void reset_passkey_cb(u32 *key);

// Complete Local Name  默认的蓝牙名字

//------------------------------------------------------
//广播参数设置
static void advertisements_setup_init();
static int set_adv_enable(void *priv, u32 en);
extern int get_buffer_vaild_len(void *priv);
extern const char *bt_get_local_name();
extern const char *bt_get_mac_addr();
extern void clr_wdt(void);
extern void sys_auto_shut_down_disable(void);
extern void sys_auto_shut_down_enable(void);
extern u8 get_total_connect_dev(void);
void bt_ble_slave_adv_enable(u8 enable);

const char *bt_get_local_name()
{
    return "bd49_test";
}

//------------------------------------------------------
#if TEST_AUDIO_DATA_UPLOAD
static const u8 test_audio_data_file[1024] = {
    1, 2, 3, 4, 5, 6, 7, 8, 9
};

#define AUDIO_ONE_PACKET_LEN  128
static void test_send_audio_data(int init_flag)
{
    static u32 send_pt = 0;
    static u32 start_flag = 0;

    if (!con_handle) {
        return;
    }

    if (init_flag) {
        log_info("audio send init\n");
        send_pt = 0;
        start_flag = 1;
    }

    if (!start_flag) {
        return;
    }

    u32 file_size = sizeof(test_audio_data_file);
    u8 *file_ptr = test_audio_data_file;

    if (send_pt >= file_size) {
        log_info("audio send Complete\n");
        start_flag = 0;
        return;
    }

    u32 send_len = file_size - send_pt;
    if (send_len > AUDIO_ONE_PACKET_LEN) {
        send_len = AUDIO_ONE_PACKET_LEN;
    }

    while (1) {
        if (app_send_user_data_check(send_len)) {
            log_info("audio send %08x\n", send_pt);
            if (app_send_user_data(ATT_CHARACTERISTIC_ae3c_01_VALUE_HANDLE, &file_ptr[send_pt], send_len, ATT_OP_AUTO_READ_CCC)) {
                log_info("audio send fail!\n");
                break;
            } else {
                send_pt += send_len;
            }
        } else {
            break;
        }
    }
}

#endif


static void send_request_connect_parameter(u8 table_index)
{
    struct conn_update_param_t *param = (void *)&connection_param_table[table_index];//static ram

    log_info("update_request:-%d-%d-%d-%d-\n", param->interval_min, param->interval_max, param->latency, param->timeout);
    if (con_handle) {
        slv_conn_param.interval_min = param->interval_min;
        slv_conn_param.interval_max = param->interval_max;
        slv_conn_param.latency = param->latency;
        slv_conn_param.timeout = param->timeout;
        ble_op_conn_param_request(con_handle, param);
    }
}

static void check_connetion_updata_deal(void)
{
    if (connection_update_enable) {
        if (connection_update_cnt < CONN_PARAM_TABLE_CNT) {
            send_request_connect_parameter(connection_update_cnt);
        }
    }
}

static void connection_update_complete_success(u8 *packet)
{
    int con_handle, conn_interval, conn_latency, conn_timeout;

    con_handle = hci_subevent_le_connection_update_complete_get_connection_handle(packet);
    conn_interval = hci_subevent_le_connection_update_complete_get_conn_interval(packet);
    conn_latency = hci_subevent_le_connection_update_complete_get_conn_latency(packet);
    conn_timeout = hci_subevent_le_connection_update_complete_get_supervision_timeout(packet);

    log_info("conn_interval = %d\n", conn_interval);
    log_info("conn_latency = %d\n", conn_latency);
    log_info("conn_timeout = %d\n", conn_timeout);

    if (conn_latency) {
        ble_op_latency_skip(con_handle, HOLD_LATENCY_CNT_MAX); //
    }

}


static void set_ble_work_state(ble_state_e state)
{
    if (state != ble_work_state) {
        log_info("ble_work_st:%x->%x\n", ble_work_state, state);
        ble_work_state = state;
        if (app_ble_state_callback) {
            app_ble_state_callback((void *)channel_priv, state);
        }
    }
}

static ble_state_e get_ble_work_state(void)
{
    return ble_work_state;
}

static void cbk_sm_packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size)
{
    sm_just_event_t *event = (void *)packet;
    u32 tmp32;
    switch (packet_type) {
    case HCI_EVENT_PACKET:
        switch (hci_event_packet_get_type(packet)) {
        case SM_EVENT_JUST_WORKS_REQUEST:
            sm_just_works_confirm(sm_event_just_works_request_get_handle(packet));
            log_info("Just Works Confirmed.\n");
            break;

        case SM_EVENT_PASSKEY_DISPLAY_NUMBER:
            log_info_hexdump(packet, size);
            memcpy(&tmp32, event->data, 4);
            log_info("Passkey display: %06u.\n", tmp32);
            break;

        case SM_EVENT_PASSKEY_INPUT_NUMBER:
            /*IO_CAPABILITY_KEYBOARD_ONLY 方式*/
            reset_passkey_cb(&tmp32);
            log_info("%04x->Passkey input: %06u.\n", event->con_handle, tmp32);
            sm_passkey_input(event->con_handle, tmp32); /*update passkey*/
            break;

        case SM_EVENT_PAIR_PROCESS:
            log_info("======PAIR_PROCESS: %02x\n", event->data[0]);
            put_buf(event->data, 4);

        }
        break;
    }
}


#if TEST_SEND_DATA_RATE
static void server_timer_handler(void)
{
    if (!con_handle) {
        test_data_count = 0;
        test_data_start = 0;
        return;
    }

    log_info("peer_rssi = %d\n", ble_vendor_get_peer_rssi(con_handle));

    if (test_data_count) {
        log_info("\n%d bytes send: %d.%02d KB/s \n", test_data_count, test_data_count / 1000, test_data_count % 1000);
        test_data_count = 0;
    }
}

static void server_timer_start(void)
{
    if (server_timer_handle) {
        return;
    }

    server_timer_handle  = sys_timer_add(NULL, server_timer_handler, 1000);
}

static void server_timer_stop(void)
{
    if (server_timer_handle) {
        sys_timeout_del(server_timer_handle);
        server_timer_handle = 0;
    }
}

void test_data_send_packet(void)
{
    u32 vaild_len = get_buffer_vaild_len(0);//获取发送buffer可写入的数据
    if (!test_data_start) {
        log_info("send buffer NULL");
        return;
    }

    if (vaild_len) {
        if (!app_send_user_data(TEST_SEND_HANDLE_VAL, (void *)&test_data_count, vaild_len, ATT_OP_AUTO_READ_CCC)) {
            test_data_count += vaild_len;
        } else {
            err_send_data_count++;
            printf("err num:%d", err_send_data_count);
        }
    }
    /* clr_wdt(); */
}
#endif


static void can_send_now_wakeup(void)
{
    /* putchar('E'); */
    if (ble_resume_send_wakeup) {
        ble_resume_send_wakeup();
    }

#if TEST_SEND_DATA_RATE
    test_data_send_packet();
#endif

#if TEST_AUDIO_DATA_UPLOAD
    test_send_audio_data(0);
#endif

}

/* static const char *const phy_result[] = { */
/*     "None", */
/*     "1M", */
/*     "2M", */
/*     "Coded", */
/* }; */

static void set_connection_data_length(u16 tx_octets, u16 tx_time)
{
    if (con_handle) {
        ble_op_set_data_length(con_handle, tx_octets, tx_time);
    }
}

static void set_connection_data_phy(u8 tx_phy, u8 rx_phy)
{
    if (0 == con_handle) {
        return;
    }

    u8 all_phys = 0;
    u16 phy_options = CONN_SET_PHY_OPTIONS_S8;

    ble_op_set_ext_phy(con_handle, all_phys, tx_phy, rx_phy, phy_options);
}

static void server_profile_start(u16 con_handle)
{
    ble_op_att_send_init(con_handle, att_ram_buffer, ATT_RAM_BUFSIZE, ATT_LOCAL_MTU_SIZE);
#if RCSP_BTMATE_EN
    void rcsp_init();
    rcsp_init();
#endif
    set_ble_work_state(BLE_ST_CONNECT);

    /* set_connection_data_phy(CONN_SET_CODED_PHY, CONN_SET_CODED_PHY); */
}

_WEAK_
u8 ble_update_get_ready_jump_flag(void)
{
    return 0;
}
/*
 * @section Packet Handler
 *
 * @text The packet handler is used to:
 *        - stop the counter after a disconnect
 *        - send a notification when the requested ATT_EVENT_CAN_SEND_NOW is received
 */

/* LISTING_START(packetHandler): Packet Handler */
static void cbk_packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size)
{
    int mtu;
    u32 tmp;
    u8 status;
    const char *attribute_name;

    switch (packet_type) {
    case HCI_EVENT_PACKET:
        switch (hci_event_packet_get_type(packet)) {

        /* case DAEMON_EVENT_HCI_PACKET_SENT: */
        /* break; */
        case ATT_EVENT_HANDLE_VALUE_INDICATION_COMPLETE:
            log_info("ATT_EVENT_HANDLE_VALUE_INDICATION_COMPLETE\n");
        case ATT_EVENT_CAN_SEND_NOW:
            can_send_now_wakeup();
            break;

        case HCI_EVENT_LE_META:
            switch (hci_event_le_meta_get_subevent_code(packet)) {
            case HCI_SUBEVENT_LE_ENHANCED_CONNECTION_COMPLETE:
                status = hci_subevent_le_enhanced_connection_complete_get_status(packet);
                if (status) {
                    log_info("LE_SLAVE CONNECTION FAIL!!! %0x\n", status);
                    set_ble_work_state(BLE_ST_DISCONN);
                    break;
                }
                con_handle = hci_subevent_le_enhanced_connection_complete_get_connection_handle(packet);
                log_info("HCI_SUBEVENT_LE_ENHANCED_CONNECTION_COMPLETE : %0x\n", con_handle);
                log_info("conn_interval = %d\n", hci_subevent_le_enhanced_connection_complete_get_conn_interval(packet));
                log_info("conn_latency = %d\n", hci_subevent_le_enhanced_connection_complete_get_conn_latency(packet));
                log_info("conn_timeout = %d\n", hci_subevent_le_enhanced_connection_complete_get_supervision_timeout(packet));
                server_profile_start(con_handle);
                break;

            case HCI_SUBEVENT_LE_CONNECTION_COMPLETE:
                con_handle = hci_subevent_le_connection_complete_get_connection_handle(packet);
                log_info("HCI_SUBEVENT_LE_CONNECTION_COMPLETE: %0x\n", con_handle);
                connection_update_complete_success(packet + 8);
                server_profile_start(con_handle);
                log_info("ble remote rssi= %d\n", ble_vendor_get_peer_rssi(con_handle));
                break;

            case HCI_SUBEVENT_LE_CONNECTION_UPDATE_COMPLETE:
                connection_update_complete_success(packet);
                if (conn_parm_update_flag & SLV_UPDATE_WAITING) {
                    conn_parm_update_flag &= ~SLV_UPDATE_WAITING;
                }
                break;

            case HCI_SUBEVENT_LE_DATA_LENGTH_CHANGE:
                log_info("APP HCI_SUBEVENT_LE_DATA_LENGTH_CHANGE\n");
                /* set_connection_data_phy(CONN_SET_CODED_PHY, CONN_SET_CODED_PHY); */
                break;

            case HCI_SUBEVENT_LE_PHY_UPDATE_COMPLETE:
                log_info("APP HCI_SUBEVENT_LE_PHY_UPDATE %s\n", hci_event_le_meta_get_phy_update_complete_status(packet) ? "Fail" : "Succ");
                printf("Tx PHY: %s\n", phy_result[hci_event_le_meta_get_phy_update_complete_tx_phy(packet)]);
                printf("Rx PHY: %s\n", phy_result[hci_event_le_meta_get_phy_update_complete_rx_phy(packet)]);

                break;
            }
            break;

        case HCI_EVENT_DISCONNECTION_COMPLETE:
            log_info("HCI_EVENT_DISCONNECTION_COMPLETE: %0x\n", packet[5]);
            con_handle = 0;
            ble_op_att_send_init(con_handle, 0, 0, 0);
            set_ble_work_state(BLE_ST_DISCONN);

            if (!ble_update_get_ready_jump_flag()) {
                bt_ble_slave_adv_enable(1);
            }
            connection_update_cnt = 0;
#if TEST_TIMER_SEND_DATA_RATE
            test_data_start = 0;//stop
#endif
            break;

        case ATT_EVENT_MTU_EXCHANGE_COMPLETE:
            mtu = att_event_mtu_exchange_complete_get_MTU(packet) - 3;
            log_info("ATT MTU = %u\n", mtu);
            ble_op_att_set_send_mtu(mtu);
            set_connection_data_length(251, 2120);
            break;

        case HCI_EVENT_VENDOR_REMOTE_TEST:
            log_info("--- HCI_EVENT_VENDOR_REMOTE_TEST\n");
            break;

        case L2CAP_EVENT_CONNECTION_PARAMETER_UPDATE_RESPONSE:
            tmp = little_endian_read_16(packet, 4);
            log_info("-update_rsp: %02x\n", tmp);
            if (tmp) {
                connection_update_cnt++;
                log_info("remoter reject!!!\n");
                check_connetion_updata_deal();
            } else {
                connection_update_cnt = CONN_PARAM_TABLE_CNT;
            }
            break;

        case HCI_EVENT_ENCRYPTION_CHANGE:
            log_info("HCI_EVENT_ENCRYPTION_CHANGE= %d\n", packet[2]);
            break;

        }
        break;
    }
}


/* LISTING_END */

/*
 * @section ATT Read
 *
 * @text The ATT Server handles all reads to constant data. For dynamic data like the custom characteristic, the registered
 * att_read_callback is called. To handle long characteristics and long reads, the att_read_callback is first called
 * with buffer == NULL, to request the total value length. Then it will be called again requesting a chunk of the value.
 * See Listing attRead.
 */

/* LISTING_START(attRead): ATT Read */

// ATT Client Read Callback for Dynamic Data
// - if buffer == NULL, don't copy data, just return size of value
// - if buffer != NULL, copy data and return number bytes copied
// @param offset defines start of attribute value
static uint16_t att_read_callback(hci_con_handle_t connection_handle, uint16_t att_handle, uint16_t offset, uint8_t *buffer, uint16_t buffer_size)
{

    uint16_t  att_value_len = 0;
    uint16_t handle = att_handle;

    log_info("read_callback, handle= 0x%04x,buffer= %08x\n", handle, (u32)buffer);

    switch (handle) {
    case ATT_CHARACTERISTIC_2a00_01_VALUE_HANDLE:
        att_value_len = gap_device_name_len;

        if ((offset >= att_value_len) || (offset + buffer_size) > att_value_len) {
            break;
        }

        if (buffer) {
            memcpy(buffer, &gap_device_name[offset], buffer_size);
            att_value_len = buffer_size;
            log_info("\n------read gap_name: %s \n", gap_device_name);
        }
        break;

    case ATT_CHARACTERISTIC_ae10_01_VALUE_HANDLE:
        att_value_len = sizeof(test_read_write_buf);
        if ((offset >= att_value_len) || (offset + buffer_size) > att_value_len) {
            break;
        }

        if (buffer) {
            memcpy(buffer, &test_read_write_buf[offset], buffer_size);
            att_value_len = buffer_size;
        }
        break;

    case ATT_CHARACTERISTIC_2a05_01_CLIENT_CONFIGURATION_HANDLE:
    case ATT_CHARACTERISTIC_ae04_01_CLIENT_CONFIGURATION_HANDLE:
    case ATT_CHARACTERISTIC_ae02_01_CLIENT_CONFIGURATION_HANDLE:
    case ATT_CHARACTERISTIC_ae05_01_CLIENT_CONFIGURATION_HANDLE:
    case ATT_CHARACTERISTIC_ae3c_01_CLIENT_CONFIGURATION_HANDLE:
        if (buffer) {
            buffer[0] = att_get_ccc_config(handle);
            buffer[1] = 0;
        }
        att_value_len = 2;
        break;

    default:
        break;
    }

    log_info("att_value_len= %d\n", att_value_len);
    return att_value_len;

}


/* LISTING_END */
/*
 * @section ATT Write
 *
 * @text The only valid ATT write in this example is to the Client Characteristic Configuration, which configures notification
 * and indication. If the ATT handle matches the client configuration handle, the new configuration value is stored and used
 * in the heartbeat handler to decide if a new value should be sent. See Listing attWrite.
 */

/* LISTING_START(attWrite): ATT Write */
static int att_write_callback(hci_con_handle_t connection_handle, uint16_t att_handle, uint16_t transaction_mode, uint16_t offset, uint8_t *buffer, uint16_t buffer_size)
{
    int result = 0;
    u16 tmp16;

    u16 handle = att_handle;
    log_info("write_callback,handle:%04x, att handle= 0x%04x,size = %d\n", connection_handle, handle, buffer_size);

    switch (handle) {

    case ATT_CHARACTERISTIC_2a00_01_VALUE_HANDLE:
        break;

    case ATT_CHARACTERISTIC_ae02_01_CLIENT_CONFIGURATION_HANDLE:
#if TEST_SEND_DATA_RATE
        if (buffer[0]) {
            server_timer_start();
        } else {
            server_timer_stop();
            test_data_start = 0;
        }
#endif

    case ATT_CHARACTERISTIC_2a05_01_CLIENT_CONFIGURATION_HANDLE:
    case ATT_CHARACTERISTIC_ae04_01_CLIENT_CONFIGURATION_HANDLE:
    case ATT_CHARACTERISTIC_ae05_01_CLIENT_CONFIGURATION_HANDLE:
    case ATT_CHARACTERISTIC_ae3c_01_CLIENT_CONFIGURATION_HANDLE:
        ble_op_latency_skip(connection_handle, HOLD_LATENCY_CNT_MIN); //
        set_ble_work_state(BLE_ST_NOTIFY_IDICATE);
        check_connetion_updata_deal();
        log_info("\n------write ccc:%04x,%02x\n", handle, buffer[0]);
        att_set_ccc_config(handle, buffer[0]);

#if TEST_SEND_DATA_RATE
        test_data_start = 1;//start
        if (buffer[0]) {
            test_data_send_packet();
        }
#endif
#if TEST_TIMER_SEND_DATA_RATE
        test_data_start = 1;//start
#endif
        break;

    case ATT_CHARACTERISTIC_ae10_01_VALUE_HANDLE:
        tmp16 = sizeof(test_read_write_buf);
        if ((offset >= tmp16) || (offset + buffer_size) > tmp16) {
            break;
        }
        memcpy(&test_read_write_buf[offset], buffer, buffer_size);
        break;

    case ATT_CHARACTERISTIC_ae01_01_VALUE_HANDLE:
        ble_op_latency_skip(connection_handle, HOLD_LATENCY_CNT_MIN); //
        /* printf("\n-ae01_rx(%d):", buffer_size); */
        /* printf_buf(buffer, buffer_size); */
        /* extern void app_ble_recv_data_api(u8 * data, u32 len); */
        /* app_ble_recv_data_api(buffer, buffer_size); */

        if (app_recieve_callback) {
            app_recieve_callback(buffer, buffer_size);
        }
#if 0
        //收发测试，自动发送收到的数据;for test
        if (app_send_user_data_check(buffer_size)) {
            app_send_user_data(ATT_CHARACTERISTIC_ae02_01_VALUE_HANDLE, buffer, buffer_size, ATT_OP_AUTO_READ_CCC);
        }

#if TEST_SEND_DATA_RATE
        if ((buffer[0] == 'A') && (buffer[1] == 'F')) {
            test_data_start = 1;//start
        } else if ((buffer[0] == 'A') && (buffer[1] == 'A')) {
            test_data_start = 0;//stop
        }
#endif
#endif
        break;

    case ATT_CHARACTERISTIC_ae03_01_VALUE_HANDLE:
        printf("\n-ae_rx(%d):", buffer_size);
        /* printf_buf(buffer, buffer_size); */

        //收发测试，自动发送收到的数据;for test
        if (app_send_user_data_check(buffer_size)) {
            app_send_user_data(ATT_CHARACTERISTIC_ae05_01_VALUE_HANDLE, buffer, buffer_size, ATT_OP_AUTO_READ_CCC);
        }
        break;
#if TRANS_DATA_EN
    case ATT_CHARACTERISTIC_ae02_02_CLIENT_CONFIGURATION_HANDLE:
        set_ble_work_state(BLE_ST_NOTIFY_IDICATE);
        log_info("\n------write ccc:%04x,%02x\n", handle, buffer[0]);
        att_set_ccc_config(handle, buffer[0]);
        break;
#endif

        /* if ((cur_conn_latency == 0) */
        /*     && (connection_update_cnt == CONN_PARAM_TABLE_CNT) */
        /*     && (Peripheral_Preferred_Connection_Parameters[0].latency != 0)) { */
        /*     connection_update_cnt = 0; */
        /* } */
        check_connetion_updata_deal();
        log_info("\n------write ccc:%04x,%02x\n", handle, buffer[0]);
        att_set_ccc_config(handle, buffer[0]);
        break;

    case ATT_CHARACTERISTIC_ae3b_01_VALUE_HANDLE:
        printf("\n-ae3b_rx(%d):", buffer_size);
        /* printf_buf(buffer, buffer_size); */

#if TEST_AUDIO_DATA_UPLOAD
        if (0 == memcmp(buffer, "start", 5)) {
            test_send_audio_data(1);
        }
#endif
        break;
#if RCSP_BTMATE_EN
    case ATT_CHARACTERISTIC_ae01_02_VALUE_HANDLE:
        /* printf("ble_rx(%d)", buffer_size); */
        /* put_buf(buffer, buffer_size > 32 ? 32 : buffer_size); */
        if (update_recieve_callback) {
            update_recieve_callback(0, buffer, buffer_size);
        }
        break;
#endif

    default:
        break;
    }

    return 0;
}

static int app_send_user_data(u16 handle, u8 *data, u16 len, u8 handle_type)
{
    u32 ret = APP_BLE_NO_ERROR;

    if (!con_handle) {
        return APP_BLE_OPERATION_ERROR;
    }

    if (!att_get_ccc_config(handle + 1)) {
        log_info("fail,no write ccc!!!,%04x\n", handle + 1);
        return APP_BLE_NO_WRITE_CCC;
    }
    ret = ble_op_att_send_data(handle, data, len, handle_type);
    if (ret == BLE_BUFFER_FULL) {
        ret = APP_BLE_BUFF_FULL;
    }

    if (ret) {
        log_info("app_send_fail:%d !!!!!!\n", ret);
    }
    return ret;
}


static int make_set_adv_data(void)
{
    u8 offset = 0;
    u8 *buf = adv_data;


    offset += make_eir_packet_val(&buf[offset], offset, HCI_EIR_DATATYPE_FLAGS, 0x06, 1);

    offset += make_eir_packet_val(&buf[offset], offset, HCI_EIR_DATATYPE_COMPLETE_16BIT_SERVICE_UUIDS, 0xAF30, 2);


    if (offset > ADV_RSP_PACKET_MAX) {
        puts("***adv_data overflow!!!!!!\n");
        return -1;
    }
    log_info("adv_data(%d):", offset);
    log_info_hexdump(buf, offset);
    adv_data_len = offset;
    ble_op_set_adv_data(offset, buf);
    return 0;
}

static int make_set_rsp_data(void)
{
    u8 offset = 0;
    u8 *buf = scan_rsp_data;


    u8 name_len = gap_device_name_len;
    u8 vaild_len = ADV_RSP_PACKET_MAX - (offset + 2);
    if (name_len > vaild_len) {
        name_len = vaild_len;
    }
    offset += make_eir_packet_data(&buf[offset], offset, HCI_EIR_DATATYPE_COMPLETE_LOCAL_NAME, (void *)gap_device_name, name_len);

    if (offset > ADV_RSP_PACKET_MAX) {
        puts("***rsp_data overflow!!!!!!\n");
        return -1;
    }

    log_info("rsp_data(%d):", offset);
    log_info_hexdump(buf, offset);
    scan_rsp_data_len = offset;
    ble_op_set_rsp_data(offset, buf);
    return 0;
}

//广播参数设置
static void advertisements_setup_init()
{
    uint8_t adv_type = ADV_IND;
    uint8_t adv_channel = ADV_CHANNEL_ALL;
    int   ret = 0;

    ble_op_set_adv_param(ADV_INTERVAL_MIN, adv_type, adv_channel);

    ret |= make_set_adv_data();
    ret |= make_set_rsp_data();

    if (ret) {
        puts("advertisements_setup_init fail !!!!!!\n");
        return;
    }

}

#define PASSKEY_ENTER_ENABLE      0 //输入passkey使能，可修改passkey
//重设passkey回调函数，在这里可以重新设置passkey
//passkey为6个数字组成，十万位、万位。。。。个位 各表示一个数字 高位不够为0
static void reset_passkey_cb(u32 *key)
{
#if 1
    u32 newkey = rand32();//获取随机数

    newkey &= 0xfffff;
    if (newkey > 999999) {
        newkey = newkey - 999999; //不能大于999999
    }
    *key = newkey; //小于或等于六位数
    printf("set new_key= %06u\n", *key);
#else
    *key = 123456; //for debug
#endif
}

static void ble_slave_sm_setup_init(io_capability_t io_type, u8 auth_req, uint8_t min_key_size, u8 security_en)
{
    //setup SM: Display only
    sm_init();
    sm_set_io_capabilities(io_type);
    sm_set_authentication_requirements(auth_req);
    sm_set_encryption_key_size_range(min_key_size, 16);
    sm_set_request_security(security_en);
    sm_event_callback_set(&cbk_sm_packet_handler);

    if (io_type == IO_CAPABILITY_DISPLAY_ONLY) {
        reset_PK_cb_register(reset_passkey_cb);
    }
}

#define TRANS_TCFG_BLE_SECURITY_EN          0/*是否发请求加密命令*/
void ble_slave_profile_init(void)
{
    printf("ble profile init\n");
    le_device_db_init();

#if PASSKEY_ENTER_ENABLE
    ble_slave_sm_setup_init(IO_CAPABILITY_DISPLAY_ONLY, SM_AUTHREQ_MITM_PROTECTION | SM_AUTHREQ_BONDING, 7, TRANS_TCFG_BLE_SECURITY_EN);
#else
    ble_slave_sm_setup_init(IO_CAPABILITY_NO_INPUT_NO_OUTPUT, SM_AUTHREQ_MITM_PROTECTION | SM_AUTHREQ_BONDING, 7, TRANS_TCFG_BLE_SECURITY_EN);
#endif

    /* setup ATT server */
    att_server_init(profile_data, att_read_callback, att_write_callback);
    att_server_register_packet_handler(cbk_packet_handler);

    // register for HCI events
    hci_event_callback_set(&cbk_packet_handler);
    le_l2cap_register_packet_handler(&cbk_packet_handler);


    ble_vendor_set_default_att_mtu(ATT_LOCAL_MTU_SIZE);
}


static int set_adv_enable(void *priv, u32 en)
{
    ble_state_e next_state, cur_state;

    if (!adv_ctrl_en && en) {
        return APP_BLE_OPERATION_ERROR;
    }

    if (con_handle) {
        return APP_BLE_OPERATION_ERROR;
    }

    if (en) {
        next_state = BLE_ST_ADV;
    } else {
        next_state = BLE_ST_IDLE;
    }

    cur_state =  get_ble_work_state();
    switch (cur_state) {
    case BLE_ST_ADV:
    case BLE_ST_IDLE:
    case BLE_ST_INIT_OK:
    case BLE_ST_NULL:
    case BLE_ST_DISCONN:
        break;
    default:
        return APP_BLE_OPERATION_ERROR;
        break;
    }

    if (cur_state == next_state) {
        return APP_BLE_NO_ERROR;
    }
    log_info("adv_en:%d\n", en);
    set_ble_work_state(next_state);

#if EXT_ADV_MODE_EN
    if (en) {
        ble_op_set_ext_adv_param(&ext_adv_param, sizeof(ext_adv_param));

        log_info_hexdump(&ext_adv_data, sizeof(ext_adv_data));
        ble_op_set_ext_adv_data(&ext_adv_data, sizeof(ext_adv_data));

        ble_op_set_ext_adv_enable(&ext_adv_enable, sizeof(ext_adv_enable));
    } else {
        ble_op_set_ext_adv_enable(&ext_adv_disable, sizeof(ext_adv_disable));
    }
#else
    if (en) {
        advertisements_setup_init();
    }
    ble_op_adv_enable(en);
#endif /* EXT_ADV_MODE_EN */

    return APP_BLE_NO_ERROR;
}

static int ble_disconnect(void *priv)
{
    if (con_handle) {
        if (BLE_ST_SEND_DISCONN != get_ble_work_state()) {
            log_info(">>>ble send disconnect\n");
            set_ble_work_state(BLE_ST_SEND_DISCONN);
            ble_op_disconnect(con_handle);
        } else {
            log_info(">>>ble wait disconnect...\n");
        }
        return APP_BLE_NO_ERROR;
    } else {
        return APP_BLE_OPERATION_ERROR;
    }
}


/* int get_buffer_vaild_len(void *priv) */
/* { */
/*     u32 vaild_len = 0; */
/*     ble_op_att_get_remain(&vaild_len); */
/*     return vaild_len; */
/* } */

static int app_send_user_data_do(void *priv, u8 *data, u16 len)
{
#if RCSP_BTMATE_EN
    return app_send_user_data(ATT_CHARACTERISTIC_ae02_02_VALUE_HANDLE, data, len, ATT_OP_AUTO_READ_CCC);
#else
    return app_send_user_data(ATT_CHARACTERISTIC_ae02_01_VALUE_HANDLE, data, len, ATT_OP_AUTO_READ_CCC);
#endif
}

static int app_send_user_data_check(u16 len)
{
    u32 buf_space = get_buffer_vaild_len(0);
    if (len <= buf_space) {
        return 1;
    }
    return 0;
}


static int regiest_wakeup_send(void *priv, void *cbk)
{
    ble_resume_send_wakeup = cbk;
    return APP_BLE_NO_ERROR;
}

static int regiest_recieve_cbk(void *priv, void *cbk)
{
    channel_priv = (u32)priv;
    update_recieve_callback = cbk;
    return APP_BLE_NO_ERROR;
}

static int regiest_state_cbk(void *priv, void *cbk)
{
    channel_priv = (u32)priv;
    app_ble_state_callback = cbk;
    return APP_BLE_NO_ERROR;
}

u8 *ble_get_scan_rsp_ptr(u16 *len)
{
    if (len) {
        *len = scan_rsp_data_len;
    }
    return scan_rsp_data;
}

u8 *ble_get_adv_data_ptr(u16 *len)
{
    if (len) {
        *len = adv_data_len;
    }
    return adv_data;
}

u8 *ble_get_gatt_profile_data(u16 *len)
{
    *len = sizeof(profile_data);
    return (u8 *)profile_data;
}


void bt_ble_slave_adv_enable(u8 enable)
{
    set_adv_enable(0, enable);
}

int get_ble_connect_handle(void)
{
    return (int)con_handle;
}
u16 bt_ble_is_connected(void)
{
    return con_handle;
}

void ble_slave_module_enable(u8 en)
{
    log_info("mode_en:%d\n", en);
    if (en) {
        adv_ctrl_en = 1;
        bt_ble_slave_adv_enable(1);
    } else {
        if (con_handle) {
            adv_ctrl_en = 0;
            ble_disconnect(NULL);
        } else {
            bt_ble_slave_adv_enable(0);
            adv_ctrl_en = 0;
        }
    }
}


//流控使能 EN: 1-停止收数 or 0-继续收数
int ble_trans_flow_enable(u8 en)
{
    int ret = -1;
#if ATT_DATA_RECIEVT_FLOW
    if (con_handle) {
        att_server_flow_hold(con_handle, en);
        ret = 0;
    }
#endif
    log_info("ble_trans_flow_enable:%d,%d\n", en, ret);
    return ret;
}

//for test
static void timer_trans_flow_test(void)
{
    static u8 sw = 0;
    if (con_handle) {
        sw = !sw;
        ble_trans_flow_enable(sw);
    }
}

static const char ble_ext_name[] = "(BLE)";

void bt_ble_slave_init(void)
{
    log_info("***** ble_init******\n");
    log_info("ble_file: %s", __FILE__);
    char *name_p;

    u8 ext_name_len = sizeof(ble_ext_name) - 1;

    name_p = bt_get_local_name();
    gap_device_name_len = strlen(name_p);
    if (gap_device_name_len > BT_NAME_LEN_MAX - ext_name_len) {
        gap_device_name_len = BT_NAME_LEN_MAX - ext_name_len;
    }

    memcpy(gap_device_name, name_p, gap_device_name_len);

    //增加后缀，区分名字
    memcpy(&gap_device_name[gap_device_name_len], "(BLE)", ext_name_len);
    gap_device_name_len += ext_name_len;
    gap_device_name[gap_device_name_len] = 0;//结束符
    log_info("ble name(%d): %s \n", gap_device_name_len, gap_device_name);
    uint8_t tmp_addr[6];
    const uint8_t *edr_addr = bt_get_mac_addr();
    le_controller_get_mac(tmp_addr);
    do {
        tmp_addr[3] = (uint8_t)rand32();
        tmp_addr[4] = (uint8_t)rand32();
        tmp_addr[5] = (uint8_t)rand32();
    } while (memcmp((void *)edr_addr, tmp_addr, 6) == 0);

    printf("ble_addr:");
    put_buf(tmp_addr, 6);
    le_controller_set_mac(tmp_addr);

#if ATT_DATA_RECIEVT_FLOW
    log_info("att_server_flow_enable\n");
    att_server_flow_enable(1);
    /* sys_timer_add(0, timer_trans_flow_test, 3000); */
#endif

    set_ble_work_state(BLE_ST_INIT_OK);
    ble_slave_module_enable(1);

#if TEST_SEND_DATA_RATE
    server_timer_start();
#endif

#if TEST_TIMER_SEND_DATA_RATE
    timer_init(0, 1);
#endif

}

void bt_ble_slave_exit(void)
{
    log_info("***** ble_exit******\n");

    ble_slave_module_enable(0);

#if TEST_SEND_DATA_RATE
    server_timer_stop();
#endif

#if TEST_TIMER_SEND_DATA_RATE
    /* timer_init(0, 1); */
#endif
}


void ble_app_disconnect(void)
{
    ble_disconnect(NULL);
}


static const struct ble_server_operation_t mi_ble_operation = {
    .adv_enable = set_adv_enable,
    .disconnect = ble_disconnect,
    .get_buffer_vaild = get_buffer_vaild_len,
    .send_data = (void *)app_send_user_data_do,
    .regist_wakeup_send = regiest_wakeup_send,
    .regist_recieve_cbk = regiest_recieve_cbk,
    .regist_state_cbk = regiest_state_cbk,
};

void ble_get_server_operation_table(struct ble_server_operation_t **interface_pt)
{
    *interface_pt = (void *)&mi_ble_operation;
}

void ble_server_send_test_key_num(u8 key_num)
{
    ;
}

int bt_ble_slave_send_api(u8 *data, u16 len)
{
    /* if (get_buffer_vaild_len(0) < len) { */
    /*     return APP_BLE_BUFF_FULL; */
    /* } */
    return app_send_user_data(ATT_CHARACTERISTIC_ae02_01_VALUE_HANDLE, data, len, ATT_OP_AUTO_READ_CCC);
}
void bt_ble_slave_recv_register(int (*callback_func)(u8 *buf, u16 len))
{
    app_recieve_callback = callback_func;
    /* extern u32 rf_receiver_deal(u8 * rf_packet, u32 packet_len); */
    /* rf_receiver_deal(data, len); */
}

u32 bt_ble_slave_update_conn_parm(u16 interval)
{
    if (!con_handle) {
        return APP_BLE_OPERATION_ERROR;
    }

    struct conn_update_param_t *param = &slv_conn_param;
    u16 last_interval = param->interval_max;
    param->interval_min = interval;
    param->interval_max = interval;
    param->latency = connection_param_table[0].latency;
    param->timeout = connection_param_table[0].timeout;
    log_info("slave connection_update_set: %04x: -%d-%d-%d-%d-\n", con_handle, \
             param->interval_min, param->interval_max, param->latency, param->timeout);
    u32 ret = ble_op_conn_param_request(con_handle, param);
    if (0 == ret) {
        u32 timeout = last_interval * 1250 * get_sys_us_cnt() * 6;//预留6次重试时间
        conn_parm_update_flag |= SLV_UPDATE_WAITING;
        while ((conn_parm_update_flag & SLV_UPDATE_WAITING) && (0 != timeout)) {
            timeout--;
        }
        if (0 == timeout) {
            /* log_info("update conn parm timeout!");  */
        }
    }
    return ret;
}

#if TEST_TIMER_SEND_DATA_RATE

#define TIMER_SFR(ch) JL_TIMER##ch

/*
 *timer
 * */
#define _timer_init(ch,ms)  \
    request_irq(IRQ_TIME##ch##_IDX, 7, timer##ch##_isr, 0); \
	SFR(TIMER_SFR(ch)->CON, 10, 4, 5);                      \
    SFR(TIMER_SFR(ch)->CON, 4, 4, 0b1000);                       \
	TIMER_SFR(ch)->CNT = 0;                                     \
	TIMER_SFR(ch)->PRD = 12000000 / 1000 / (1 * 256) * ms;	       \
	TIMER_SFR(ch)->CON |= BIT(14);                              \
	TIMER_SFR(ch)->CON |= BIT(0);


SET(interrupt(""))
static void timer0_isr(void)
{
    TIMER_SFR(0)->CON |= BIT(14);

    if (test_data_start) {
        if (!app_send_user_data(TEST_SEND_HANDLE_VAL, (void *)&send_buffer_test, 10, ATT_OP_AUTO_READ_CCC)) {
            send_buffer_test[0]++;
            putchar('@');//succ
        } else {
            err_send_data_count++;
            log_info("err num:%d", err_send_data_count);//fail
        }
    }
    /* JL_PORTA->DIR &= ~BIT(7); */
    /* JL_PORTA->OUT ^= BIT(7); */
    /* log_char('0'); */
}
SET(interrupt(""))
void timer1_isr(void)
{
    TIMER_SFR(1)->CON |= BIT(14);
    log_char('1');
}
SET(interrupt(""))
static void timer2_isr(void)
{
    TIMER_SFR(2)->CON |= BIT(14);
    log_char('2');
}

static void timer_init(u8 timer_ch, u32 ms)
{
    switch (timer_ch) {
    case 0:
        _timer_init(0, ms);
        break;
    case 1:
        _timer_init(1, ms);
        break;
    case 2:
        _timer_init(2, ms);
        break;
    default:
        break;
    }
}
#endif

const vble_smpl_role_ops ble_slave_ops = {
    .name = "ble_slave",
    .init = bt_ble_slave_init,
    .exit =  bt_ble_slave_exit,
    .send = bt_ble_slave_send_api,
    .recv_cb_register = bt_ble_slave_recv_register,
    .module_enable = ble_slave_module_enable,
    .adv_enable = bt_ble_slave_adv_enable,
    .profile_init = ble_slave_profile_init,
    .get_ble_status = get_ble_work_state,
    .update_conn_interval = bt_ble_slave_update_conn_parm,
};

#endif
#endif

