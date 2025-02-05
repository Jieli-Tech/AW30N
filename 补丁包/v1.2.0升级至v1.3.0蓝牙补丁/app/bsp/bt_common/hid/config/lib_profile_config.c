#include "includes.h"
#include "hid_app_config.h"
#include "btcontroller_config.h"
#include "bt_profile_cfg.h"
/* #include "bt_common.h" */
#include "avctp_user.h"

#ifdef SUPPORT_MS_EXTENSIONS
#pragma bss_seg(".ble_app_bss")
#pragma data_seg(".ble_app_data")
#pragma const_seg(".ble_app_text_const")
#pragma code_seg(".ble_app_text")
#endif

#if (TRANS_DATA_HID_EN)
#define LOG_TAG     "[BT-CFG]"
#define LOG_ERROR_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
/* #include "debug.h" */
#include "log.h"


typedef struct {
    // linked list - assert: first field
    void *offset_item;

    // data is contained in same memory
    u32        service_record_handle;
    u8         *service_record;
} service_record_item_t;

extern const u8 sdp_hfp_service_data[];
/* extern const u8 sdp_pnp_service_data_for_hid[]; */
extern const u8 sdp_pnp_service_data[];
extern const u8 sdp_spp_service_data[];
extern service_record_item_t  sdp_record_item_begin[];
extern service_record_item_t  sdp_record_item_end[];

#define SDP_RECORD_HANDLER_REGISTER(handler) \
	const service_record_item_t  handler \
		sec(.sdp_record_item)

#if TCFG_USER_BLE_ENABLE

#if (TCFG_BLE_DEMO_SELECT == DEF_BLE_DEMO_ADV)
const int config_stack_modules = (BT_BTSTACK_LE_ADV) | BT_BTSTACK_CLASSIC;
#elif (TCFG_BLE_DEMO_SELECT == DEF_BLE_DEMO_MESH)
const int config_stack_modules = BT_BTSTACK_LE;
#else

const int config_stack_modules = BT_BTSTACK_LE;
#endif

#else

const int config_stack_modules = 0;
#endif

//定义的产品信息,for test
#define  PNP_VID_SOURCE   0x02
#define  PNP_VID          0x05ac //0x05d6
#define  PNP_PID          0x022C //
#define  PNP_PID_VERSION  0x011b //1.1.11

/* static const u8 sdp_pnp_service_data_hid[] = { */
/* 0x36, 0x00, 0x5a, 0x09, 0x00, 0x00, 0x0a, 0x00, 0x01, 0x00, 0x0a, 0x09, 0x00, 0x01, 0x35, 0x03, */
/* 0x19, 0x12, 0x00, 0x09, 0x00, 0x04, 0x35, 0x0d, 0x35, 0x06, 0x19, 0x01, 0x00, 0x09, 0x00, 0x01, */
/* 0x35, 0x03, 0x19, 0x00, 0x01, 0x09, 0x00, 0x05, 0x35, 0x03, 0x19, 0x10, 0x02, 0x09, 0x00, 0x09, */
/* 0x35, 0x08, 0x35, 0x06, 0x19, 0x12, 0x00, 0x09, 0x01, 0x00, 0x09, 0x02, 0x00, 0x09, 0x01, 0x03, */
/* 0x09, 0x02, 0x01, 0x09, PNP_VID >> 8, PNP_VID & 0xFF, 0x09, 0x02, 0x02, 0x09, PNP_PID >> 8, PNP_PID & 0xFF, */
/* 0x09, 0x02, 0x03, 0x09, */
/* PNP_PID_VERSION >> 8, PNP_PID_VERSION & 0xFF, 0x09, 0x02, 0x04, 0x28, 0x01, 0x09, 0x02, 0x05, 0x09, */
/* PNP_VID_SOURCE >> 8, PNP_VID_SOURCE & 0xFF, 0x00, 0x00 */
/* }; */

static const u8 sdp_pnp_service_data_hid[] = {
    0x36, 0x00, 0xA1, 0x09, 0x00, 0x00, 0x0A, 0x00, 0x01, 0x00, 0x01, 0x09, 0x00, 0x01, 0x35, 0x03,
    0x19, 0x12, 0x00, 0x09, 0x00, 0x04, 0x35, 0x0D, 0x35, 0x06, 0x19, 0x01, 0x00, 0x09, 0x00, 0x01,
    0x35, 0x03, 0x19, 0x00, 0x01, 0x09, 0x00, 0x06, 0x35, 0x09, 0x09, 0x65, 0x6E, 0x09, 0x00, 0x6A,
    0x09, 0x01, 0x00, 0x09, 0x00, 0x09, 0x35, 0x08, 0x35, 0x06, 0x19, 0x12, 0x00, 0x09, 0x01, 0x00,
    0x09, 0x01, 0x00, 0x25, 0x2F, 0x42, 0x72, 0x6F, 0x61, 0x64, 0x63, 0x6F, 0x6D, 0x20, 0x42, 0x6C,
    0x75, 0x65, 0x74, 0x6F, 0x6F, 0x74, 0x68, 0x20, 0x57, 0x69, 0x72, 0x65, 0x6C, 0x65, 0x73, 0x73,
    0x20, 0x4B, 0x65, 0x79, 0x62, 0x6F, 0x61, 0x72, 0x64, 0x20, 0x50, 0x6E, 0x50, 0x20, 0x53, 0x65,
    0x72, 0x76, 0x65, 0x72, 0x09, 0x01, 0x01, 0x25, 0x08, 0x4B, 0x65, 0x79, 0x62, 0x6F, 0x61, 0x72,
    0x64, 0x09, 0x02, 0x00, 0x09, 0x01, 0x03, 0x09, 0x02, 0x01, 0x09, PNP_VID >> 8, PNP_VID & 0xFF, 0x09, 0x02, 0x02,
    0x09, PNP_PID >> 8, PNP_PID & 0xFF, 0x09, 0x02, 0x03, 0x09, PNP_PID_VERSION >> 8, PNP_PID_VERSION & 0xFF, 0x09, 0x02, 0x04, 0x28, 0x01, 0x09, 0x02,
    0x05, 0x09, PNP_VID_SOURCE >> 8, PNP_VID_SOURCE & 0xFF, 0x00, 0x00, 0x00
};

/*重定义下面hid信息结构，信息用于提供给库里面接口SDP生成hid_service服务使用*/
/*用到结构体的两个接口:sdp_create_diy_device_ID_service 和 sdp_create_diy_hid_service*/
static const hid_sdp_info_t hid_sdp_info_config = {
    .vid_private = PNP_VID,
    .pid_private = PNP_PID,
    .ver_private = PNP_PID_VERSION,

    .sub_class           = 0x80,
    .country_code        = 0x21,
    .virtual_cable       = 0x01,
    .reconnect_initiate  = 0x01,
    .sdp_disable         = 0x00,
    .battery_power       = 0x01,
    .remote_wake         = 0x01,
    .normally_connectable = 0x01,
    .boot_device         = 0x01,
    .version             = 0x0100,
    .parser_version      = 0x0111,
    .profile_version     = 0x0100,
    .supervision_timeout = 0x1f40,
    .language            = 0x0409,
    .bt_string_offset    = 0x0100,
    .descriptor_len      = 0,
    .descriptor          = NULL,
    .service_name        = "JL_HID",
    .service_description = "hid key",
    .provide_name        = "JIELI",
    .sdp_request_respone_callback = NULL,
    .extra_buf              = NULL,
    .extra_len              = 0,
};


#define NEW_SDP_PNP_DATA_VER     1  //for 兼容性

#if (USER_SUPPORT_PROFILE_HID==1)
u8 sdp_make_pnp_service_data[0x60];
SDP_RECORD_HANDLER_REGISTER(pnp_sdp_record_item) = {

#if NEW_SDP_PNP_DATA_VER
    .service_record = (u8 *)sdp_pnp_service_data_hid,
#else
    .service_record = (u8 *)sdp_make_pnp_service_data,
#endif
    .service_record_handle = 0x1000A,
};
#else
SDP_RECORD_HANDLER_REGISTER(pnp_sdp_record_item) = {
    .service_record = (u8 *)sdp_pnp_service_data,
    .service_record_handle = 0x1000A,
};
#endif


#if (USER_SUPPORT_PROFILE_HID==1)
u8 hid_profile_support = 1;
u8 sdp_make_hid_service_data[0x200];
SDP_RECORD_HANDLER_REGISTER(hid_sdp_record_item) = {
    .service_record = (u8 *)sdp_make_hid_service_data,
    .service_record_handle = 0x00010006,
};
#endif

#if (USER_SUPPORT_PROFILE_HFP==1)
u8 hfp_profile_support = 1;
SDP_RECORD_HANDLER_REGISTER(hfp_sdp_record_item) = {
    .service_record = (u8 *)sdp_hfp_service_data,
    .service_record_handle = 0x00010003,
};
#endif

void hid_sdp_init(const u8 *hid_descriptor, u16 size)
{
#if (USER_SUPPORT_PROFILE_HID==1)
    int real_size;
    /*reset info config,在生成sdp数组接口前配置*/
    sdp_diy_set_config_hid_info(&hid_sdp_info_config);

#if (NEW_SDP_PNP_DATA_VER == 0)
    real_size = sdp_create_diy_device_ID_service(sdp_make_pnp_service_data, sizeof(sdp_make_pnp_service_data));
    printf("dy_device_id_service(%d):", real_size);
#endif
    /* put_buf(sdp_make_pnp_service_data,real_size); */

    real_size = sdp_create_diy_hid_service(sdp_make_hid_service_data, sizeof(sdp_make_hid_service_data), hid_descriptor, size);
    printf("dy_hid_service(%d):", real_size);
    /* put_buf(sdp_make_hid_service_data,real_size); */
#endif
}

/*注意hid_conn_depend_on_dev_company置2之后，默认不断开HID连接 */
const u8 hid_conn_depend_on_dev_company = 2;
const u8 sdp_get_remote_pnp_info = 0;


#if ((TCFG_USER_BLE_ENABLE) && (TCFG_BLE_DEMO_SELECT != DEF_BLE_DEMO_ADV))
u8 app_le_pool[900] sec(.btstack_pool)  ALIGNED(4);
#endif

u8 app_bredr_pool[0] sec(.btstack_pool) ALIGNED(4);
u8 app_bredr_profile[0] sec(.btstack_pool) ALIGNED(4);

#if TCFG_USER_BLE_ENABLE
u8 app_l2cap_pool[70] sec(.btstack_pool) ALIGNED(4);
#endif

u8 *get_bredr_pool_addr(void)
{
    u16 len = 0;

    if (STACK_MODULES_IS_SUPPORT(BT_BTSTACK_CLASSIC)) {
        extern u16 get_bredr_pool_len(void);
        len = get_bredr_pool_len();
        printf("bredr pool len %d\n", len);
        if (len > sizeof(app_bredr_pool)) {
            ASSERT(0, "bredr_pool is small\n");
        }
        return &app_bredr_pool;
    }
    return NULL;
}

u8 *get_le_pool_addr(void)
{
    u16 len = 0;

#if ((TCFG_USER_BLE_ENABLE) && (TCFG_BLE_DEMO_SELECT != DEF_BLE_DEMO_ADV))
    if (STACK_MODULES_IS_SUPPORT(BT_BTSTACK_LE)) {
        extern u16 get_le_pool_len(void);
        len = get_le_pool_len();
        printf("le pool len %d\n", len);
        if (len > sizeof(app_le_pool)) {
            ASSERT(0, "le_pool is small\n");
        }

        return &app_le_pool;
    }
#endif
    return NULL;
}

u8 *get_l2cap_stack_addr(void)
{
    u16 len = 0;

    if (STACK_MODULES_IS_SUPPORT(BT_BTSTACK_CLASSIC) || STACK_MODULES_IS_SUPPORT(BT_BTSTACK_LE)) {
        extern u16 get_l2cap_stack_len(void);
        len = get_l2cap_stack_len();
        printf("l2cap stack len %d\n", len);
        if (len > sizeof(app_l2cap_pool)) {
            ASSERT(0, "l2cap pool is small\n");
        }
        return &app_l2cap_pool;
    } else {
        return NULL;
    }
}

u8 *get_profile_pool_addr(void)
{
    u16 len = 0;

    if (STACK_MODULES_IS_SUPPORT(BT_BTSTACK_CLASSIC)) {

        extern u16 get_profile_pool_len(void);
        len = get_profile_pool_len();
        printf("bredr profile pool len %d\n", len);
        if (len > sizeof(app_bredr_profile)) {
            ASSERT(0, "bredr_profile is small\n");
        }
        return &app_bredr_profile;
    }
    return NULL;
}

const u8 a2dp_mutual_support = 0;
const u8 more_addr_reconnect_support = 0;
const u8 more_hfp_cmd_support = 0;
const u8 more_avctp_cmd_support = 0;
const u8 hci_inquiry_support = 0;
const u8 btstack_emitter_support  = 0;  /*定义用于优化代码编译*/
const u8 adt_profile_support = 0;
const u8 pbg_support_enable = 0;



/*u8 l2cap_debug_enable = 0xf0;
u8 rfcomm_debug_enable = 0xf;
u8 profile_debug_enable = 0xff;
u8 ble_debug_enable    = 0xff;
u8 btstack_tws_debug_enable = 0xf;*/
#endif
