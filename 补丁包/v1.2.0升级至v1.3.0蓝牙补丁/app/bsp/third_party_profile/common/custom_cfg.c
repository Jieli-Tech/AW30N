#include "custom_cfg.h"
#include "vfs.h"
#include "app_config.h"
#include "rcsp_bluetooth.h"
#include "crc16.h"
#include "ioctl_cmds.h"

#define LOG_TAG_CONST      RCSP_MODE
#define LOG_TAG             "[APP_CUSTOM]"
#include "log.h"

#if RCSP_BTMATE_EN

#define SECOTR_SIZE (4*1024L)
#define RES_CUSTOM_CFG_FILE "/config.dat"
#define EXIF_CFG_PATH   "/app_area_head/EXIF"

//配置:是否在总是擦除EXIF区域然后重写,还是只有在EXIF信息有变化时才擦除
#define ALWAYS_ERASE_EXIF_AREA      0
#define ONLY_DIFF_ERASE_EXIF_AREA   1
#define EXIF_ERASE_CONFIG           ALWAYS_ERASE_EXIF_AREA

#define MEMBER_OFFSET_OF_STRUCT(type, member)       ((u32)&(((type *)0)->member))
#define MEMBER_SIZE_OF_STRUCT(type,member)          ((u16)sizeof(((type *)0)->member))

#define fopen vfs_openbypath
#define fread vfs_read
#define fseek vfs_seek
#define fclose vfs_file_close

typedef enum _FLASH_ERASER {
    CHIP_ERASER,
    BLOCK_ERASER,
    SECTOR_ERASER,
} FLASH_ERASER;

typedef enum _cfg_err_type_t {
    CFG_ERR_NONE = 0,
    CFG_ERR_FILE_INDEX_ERR,
    CFG_ERR_FILE_SEEK_ERR,
    CFG_ERR_FILE_READ_ERR,
    CFG_ERR_ITEM_LEN_OVER,
    CFG_ERR_ITEM_NO_FOUND,
} cfg_err_type_t;

extern int norflash_erase(u32 cmd, u32 addr);
extern int sfc_norflash_read(struct device *device, void *buf, u32 len, u32 offset);
extern int sfc_norflash_write(struct device *device, void *buf, u32 len, u32 offset);
extern const char *bt_get_local_name();
extern const u8 *bt_get_mac_addr();
extern u8 *ble_get_scan_rsp_ptr(u16 *len);
extern u8 *ble_get_adv_data_ptr(u16 *len);
extern u8 *ble_get_gatt_profile_data(u16 *len);
extern u8 *get_last_device_connect_linkkey(u16 *len);

typedef struct _exif_info_t {
    u32 addr;
    u32 len;
} exif_info_t;

typedef struct _adv_data_t {
    u16 crc;
    u16 len;
    u8 data[31];
} adv_data_cfg_t;

typedef struct _ble_name_t {
    u16 crc;
    u16 len;
    u8 data[31 - 2];
} ble_name_t;

typedef struct _bt_name_t {
    u16 crc;
    u16 len;
    u8 data[31];
} bt_name_t;

typedef struct _bt_pin_code_t {
    u16 crc;
    u16 len;
    char data[4];
} bt_pin_code_t;

typedef struct _ver_info_cfg_t {
    u16 crc;
    u16 len;
    update_file_id_t data;
} ver_info_cfg_t;

typedef struct LOW_POWER_VOLTAGE {
    u16 crc;
    u16 len;
    u8 data[2];
} low_power_voltage_t;

typedef struct _bt_addr_cfg_t {
    u16 crc;
    u16 len;
    u8 data[6];
} bt_addr_cfg_t;

typedef struct _gatt_profile_cfg_t {
    u16 crc;
    u16 len;
    u8 data[512 + 256];
} gatt_profile_cfg_t;

typedef struct _reset_io_info_cfg_t {
    u16 crc;
    u16 len;
    u8 data;
} reset_io_info_cfg_t;

typedef struct _polit_lamp_io_info_cfg_t {
    u16 crc;
    u16 len;
    u8 data[4];
} pilot_lamp_io_info_cfg_t;

typedef struct _link_key_info_cfg_t {
    u16 crc;
    u16 len;
    update_file_link_key_t data;
} link_key_info_cfg_t;

typedef struct _power_io_on_off_cfg_t {
    u16 crc;
    u16 len;
    u8 data[6];
} power_io_on_off_cfg_t;

typedef struct _last_device_connect_linkkey_cfg_t {
    u16 crc;
    u16 len;
    u8 data[16];
} last_device_connect_linkkey_cfg_t;

typedef struct _read_write_uuid_cfg_t {
    u16 crc;
    u16 len;
    u8 data[4];
} read_write_uuid_cfg_t;

#if VER_INFO_EXT_COUNT
typedef struct _ver_info_ext_cfg_t {
    u16 crc;
    u16 len;
    u8 data[VER_INFO_EXT_MAX_LEN];
} ver_info_ext_cfg_t;
#endif

typedef struct _pid_vid_cfg_t {
    u16 crc;
    u16 len;
    u8 data[4];
} pid_vid_cfg_t;

typedef struct _md5_cfg_t {
    u16 crc;
    u16 len;
    u8 data[32];
} md5_cfg_t;

typedef struct _sdk_type_cfg_t {
    u16 crc;
    u16 len;
    u8 data;
} sdk_type_cfg_t;

typedef struct _ex_cfg_t {
    adv_data_cfg_t adv_data_cfg;
    adv_data_cfg_t scan_rsp_cfg;
    ble_name_t ble_name_cfg;
    bt_name_t bt_name_cfg;
    bt_pin_code_t pin_code_cfg;
    ver_info_cfg_t ver_info_cfg;
    low_power_voltage_t low_power_voltage_cfg;
    bt_addr_cfg_t edr_addr_cfg;
    bt_addr_cfg_t ble_addr_cfg;
    gatt_profile_cfg_t gatt_profile_cfg;
    reset_io_info_cfg_t reset_io_info_cfg;
    pilot_lamp_io_info_cfg_t pilot_lamp_io_info_cfg;
    link_key_info_cfg_t link_key_info_cfg;
    power_io_on_off_cfg_t power_io_on_off_cfg;
    last_device_connect_linkkey_cfg_t last_device_connect_linkkey_cfg;
    read_write_uuid_cfg_t ble_read_write_uuid_cfg;
#if VER_INFO_EXT_COUNT
    ver_info_ext_cfg_t ver_info_authkey_cfg;
    ver_info_ext_cfg_t ver_info_procode_cfg;
#endif
    pid_vid_cfg_t       pid_vid_cfg;
    md5_cfg_t           md5_cfg;
    sdk_type_cfg_t      sdk_type_cfg;
} ex_cfg_t;

typedef struct _common_cfg_t {
    u16 crc;
    u16 len;
    u8 data[0];
} common_cfg_t;

typedef struct _cfg_item_attr_t {
    u16 item_len;
    u16 data_len;
    u16 member_offset;
} cfg_item_attr_t;

typedef struct _ex_cfg_item_u {
    adv_data_cfg_t adv_data_cfg;
    adv_data_cfg_t scan_rsp_cfg;
    ble_name_t ble_name_cfg;
    bt_name_t bt_name_cfg;
    bt_pin_code_t pin_code_cfg;
    ver_info_cfg_t ver_info_cfg;
    low_power_voltage_t low_power_voltage_cfg;
    bt_addr_cfg_t edr_addr_cfg;
    bt_addr_cfg_t ble_addr_cfg;
    gatt_profile_cfg_t gatt_profile_cfg;
    reset_io_info_cfg_t reset_io_info_cfg;
    pilot_lamp_io_info_cfg_t pilot_lamp_io_info_cfg;
    link_key_info_cfg_t link_key_info_cfg;
    power_io_on_off_cfg_t power_io_on_off_cfg;
    last_device_connect_linkkey_cfg_t last_device_connect_linkkey_cfg;
    read_write_uuid_cfg_t ble_read_write_uuid_cfg;
#if VER_INFO_EXT_COUNT
    ver_info_ext_cfg_t ver_info_authkey_cfg;
    ver_info_ext_cfg_t ver_info_procode_cfg;
#endif
    common_cfg_t common_cfg;
    pid_vid_cfg_t       pid_vid_cfg;
    md5_cfg_t           md5_cfg;
    sdk_type_cfg_t      sdk_type_cfg;
} ex_cfg_item_u;

typedef struct _cfg_item_head_t {
    u16 index;
    u16 type;
    u32 addr;
    u32 len;
    u16 crc;
    u16 rfu;
    u8 name[16];
} cfg_item_head_t;

typedef struct _cfg_head_t {
    u8 flag[4];
    u16 self_crc;
    u16 item_head_crc;
    u32 len;
    u16 item_count;
    u16 rfu;
    u8 name[16];
} cfg_head_t;

static const cfg_item_attr_t cfg_item_attr_tab[] = {
    [CFG_ITEM_ADV_IND] = {
        MEMBER_SIZE_OF_STRUCT(ex_cfg_t, adv_data_cfg),
        MEMBER_SIZE_OF_STRUCT(ex_cfg_t, adv_data_cfg.data),
        MEMBER_OFFSET_OF_STRUCT(ex_cfg_t, adv_data_cfg),
    },
    [CFG_ITEM_SCAN_RSP] = {
        MEMBER_SIZE_OF_STRUCT(ex_cfg_t, scan_rsp_cfg),
        MEMBER_SIZE_OF_STRUCT(ex_cfg_t, scan_rsp_cfg.data),
        MEMBER_OFFSET_OF_STRUCT(ex_cfg_t, scan_rsp_cfg),
    },
    [CFG_ITEM_BLE_NAME] = {
        MEMBER_SIZE_OF_STRUCT(ex_cfg_t, ble_name_cfg),
        MEMBER_SIZE_OF_STRUCT(ex_cfg_t, ble_name_cfg.data),
        MEMBER_OFFSET_OF_STRUCT(ex_cfg_t, ble_name_cfg),
    },
    [CFG_ITEM_BT_NAME] = {
        MEMBER_SIZE_OF_STRUCT(ex_cfg_t, bt_name_cfg),
        MEMBER_SIZE_OF_STRUCT(ex_cfg_t, bt_name_cfg.data),
        MEMBER_OFFSET_OF_STRUCT(ex_cfg_t, bt_name_cfg),
    },
    [CFG_ITEM_PIN_CODE] = {
        MEMBER_SIZE_OF_STRUCT(ex_cfg_t, pin_code_cfg),
        MEMBER_SIZE_OF_STRUCT(ex_cfg_t, pin_code_cfg.data),
        MEMBER_OFFSET_OF_STRUCT(ex_cfg_t, pin_code_cfg),
    },
    [CFG_ITEM_VER_INFO] = {
        MEMBER_SIZE_OF_STRUCT(ex_cfg_t, ver_info_cfg),
        MEMBER_SIZE_OF_STRUCT(ex_cfg_t, ver_info_cfg.data),
        MEMBER_OFFSET_OF_STRUCT(ex_cfg_t, ver_info_cfg),
    },
    [CFG_ITEM_LOW_POWER_VOLTAGE] = {
        MEMBER_SIZE_OF_STRUCT(ex_cfg_t, low_power_voltage_cfg),
        MEMBER_SIZE_OF_STRUCT(ex_cfg_t, low_power_voltage_cfg.data),
        MEMBER_OFFSET_OF_STRUCT(ex_cfg_t, low_power_voltage_cfg),
    },
    [CFG_ITEM_EDR_ADDR] = {
        MEMBER_SIZE_OF_STRUCT(ex_cfg_t, edr_addr_cfg),
        MEMBER_SIZE_OF_STRUCT(ex_cfg_t, edr_addr_cfg.data),
        MEMBER_OFFSET_OF_STRUCT(ex_cfg_t, edr_addr_cfg),
    },
    [CFG_ITEM_BLE_ADDR] = {
        MEMBER_SIZE_OF_STRUCT(ex_cfg_t, ble_addr_cfg),
        MEMBER_SIZE_OF_STRUCT(ex_cfg_t, ble_addr_cfg.data),
        MEMBER_OFFSET_OF_STRUCT(ex_cfg_t, ble_addr_cfg),
    },
    [CFG_ITEM_GATT_PROFILE] = {
        MEMBER_SIZE_OF_STRUCT(ex_cfg_t, gatt_profile_cfg),
        MEMBER_SIZE_OF_STRUCT(ex_cfg_t, gatt_profile_cfg.data),
        MEMBER_OFFSET_OF_STRUCT(ex_cfg_t, gatt_profile_cfg),
    },
    [CFG_ITEM_RESET_IO_INFO] = {
        MEMBER_SIZE_OF_STRUCT(ex_cfg_t, reset_io_info_cfg),
        MEMBER_SIZE_OF_STRUCT(ex_cfg_t, reset_io_info_cfg.data),
        MEMBER_OFFSET_OF_STRUCT(ex_cfg_t, reset_io_info_cfg),
    },
    [CFG_ITEM_PILOT_LAMP_IO_INFO] = {
        MEMBER_SIZE_OF_STRUCT(ex_cfg_t, pilot_lamp_io_info_cfg),
        MEMBER_SIZE_OF_STRUCT(ex_cfg_t, pilot_lamp_io_info_cfg.data),
        MEMBER_OFFSET_OF_STRUCT(ex_cfg_t, pilot_lamp_io_info_cfg),
    },
    [CFG_ITEM_LINK_KEY_INFO] = {
        MEMBER_SIZE_OF_STRUCT(ex_cfg_t, link_key_info_cfg),
        MEMBER_SIZE_OF_STRUCT(ex_cfg_t, link_key_info_cfg.data),
        MEMBER_OFFSET_OF_STRUCT(ex_cfg_t, link_key_info_cfg),
    },
    [CFG_ITEM_POWER_IO_ON_OFF] = {
        MEMBER_SIZE_OF_STRUCT(ex_cfg_t, power_io_on_off_cfg),
        MEMBER_SIZE_OF_STRUCT(ex_cfg_t, power_io_on_off_cfg.data),
        MEMBER_OFFSET_OF_STRUCT(ex_cfg_t, power_io_on_off_cfg),
    },
    [CFG_ITEM_LAST_DEVICE_LINK_KEY_INFO] = {
        MEMBER_SIZE_OF_STRUCT(ex_cfg_t, last_device_connect_linkkey_cfg),
        MEMBER_SIZE_OF_STRUCT(ex_cfg_t, last_device_connect_linkkey_cfg.data),
        MEMBER_OFFSET_OF_STRUCT(ex_cfg_t, last_device_connect_linkkey_cfg),
    },
    [CFG_ITEM_BLE_READ_WRITE_UUID_INFO] = {
        MEMBER_SIZE_OF_STRUCT(ex_cfg_t, ble_read_write_uuid_cfg),
        MEMBER_SIZE_OF_STRUCT(ex_cfg_t, ble_read_write_uuid_cfg.data),
        MEMBER_OFFSET_OF_STRUCT(ex_cfg_t, ble_read_write_uuid_cfg),
    },
#if VER_INFO_EXT_COUNT
    [CFG_ITEM_VER_INFO_AUTHKEY] = {
        MEMBER_SIZE_OF_STRUCT(ex_cfg_t, ver_info_authkey_cfg),
        MEMBER_SIZE_OF_STRUCT(ex_cfg_t, ver_info_authkey_cfg.data),
        MEMBER_OFFSET_OF_STRUCT(ex_cfg_t, ver_info_authkey_cfg),
    },
    [CFG_ITEM_VER_INFO_PROCODE] = {
        MEMBER_SIZE_OF_STRUCT(ex_cfg_t, ver_info_procode_cfg),
        MEMBER_SIZE_OF_STRUCT(ex_cfg_t, ver_info_procode_cfg.data),
        MEMBER_OFFSET_OF_STRUCT(ex_cfg_t, ver_info_procode_cfg),
    },
#endif
    [CFG_ITEM_PVID] = {
        MEMBER_SIZE_OF_STRUCT(ex_cfg_t, pid_vid_cfg),
        MEMBER_SIZE_OF_STRUCT(ex_cfg_t, pid_vid_cfg.data),
        MEMBER_OFFSET_OF_STRUCT(ex_cfg_t, pid_vid_cfg),
    },
    [CFG_ITEM_MD5] = {
        MEMBER_SIZE_OF_STRUCT(ex_cfg_t, md5_cfg),
        MEMBER_SIZE_OF_STRUCT(ex_cfg_t, md5_cfg.data),
        MEMBER_OFFSET_OF_STRUCT(ex_cfg_t, md5_cfg),
    },
    [CFG_ITEM_SDK_TYPE] = {
        MEMBER_SIZE_OF_STRUCT(ex_cfg_t, sdk_type_cfg),
        MEMBER_SIZE_OF_STRUCT(ex_cfg_t, sdk_type_cfg.data),
        MEMBER_OFFSET_OF_STRUCT(ex_cfg_t, sdk_type_cfg),
    }
};

typedef struct _cfg_item_description {
    u8 *item_name;
    u8 *item_data;
    u16 item_len;
    u16 *real_len;
} cfg_item_description_t;

typedef struct _update_file_id_cfg {
    u16 len;
    update_file_id_t data;
} update_file_id_cfg_t;

static update_file_id_cfg_t update_id_cfg = {
    .len = 0,
};

typedef struct _update_file_reset_io_cfg {
    u16 len;
    update_file_reset_io_t data;
} update_file_reset_io_cfg_t;

static update_file_reset_io_cfg_t update_reset_io_cfg = {
    .len = 0,
};

typedef struct _update_file_pilot_lamp_io_cfg {
    u16 len;
    update_file_pilot_lamp_io_t data;
} update_file_pilot_lamp_io_cfg_t;

static update_file_pilot_lamp_io_cfg_t update_pilot_lamp_io_cfg = {
    .len = 0,
};

typedef struct _update_file_link_key_cfg {
    u16 len;
    update_file_link_key_t data;
} update_file_link_key_cfg_t;

static update_file_link_key_cfg_t update_link_cfg = {
    .len = 0,
};

typedef struct _update_file_power_io_on_off_cfg {
    u16 len;
    u8 data[16];
} update_file_power_io_on_off_cfg_t;

static update_file_power_io_on_off_cfg_t update_power_io_on_off_cfg = {
    .len = 0,
};

#if VER_INFO_EXT_COUNT
typedef struct _update_file_ver_info_ext_cfg {
    u16 len;
    u8 data[VER_INFO_EXT_COUNT * (VER_INFO_EXT_MAX_LEN + 1)];       // authkey + , + procode + '\0'
} update_file_ver_info_ext_cfg_t;

static update_file_ver_info_ext_cfg_t update_file_ver_info_ext_cfg = {
    .len = 0,
};
#endif

static const cfg_item_description_t cfg_item_description[] = {
    [0] = {
        (u8 *)"ver_info", (u8 *) &update_id_cfg.data, sizeof(update_id_cfg.data), &update_id_cfg.len
    },
    [1] = {
        (u8 *)"reset_io", (u8 *) &update_reset_io_cfg.data, sizeof(update_reset_io_cfg.data), &update_reset_io_cfg.len
    },
    [2] = {
        (u8 *)"pilot_lamp_io", (u8 *) &update_pilot_lamp_io_cfg.data, sizeof(update_pilot_lamp_io_cfg.data), &update_pilot_lamp_io_cfg.len
    },
    [3] = {
        (u8 *)"link_key", (u8 *) &update_link_cfg.data, sizeof(update_link_cfg.data), &update_link_cfg.len
    },
    [4] = {
        (u8 *)"power_io", (u8 *) &update_power_io_on_off_cfg.data, sizeof(update_power_io_on_off_cfg.data), &update_power_io_on_off_cfg.len
    },
#if VER_INFO_EXT_COUNT
    [5] = {
        (u8 *)"ver_info_ext", (u8 *) &update_file_ver_info_ext_cfg.data, sizeof(update_file_ver_info_ext_cfg.data), &update_file_ver_info_ext_cfg.len
    },
#endif
};

static exif_info_t exif_info;

static void ex_cfg_get_addr_and_len(u32 *addr, u32 *len)
{
    u32 err = 0;
    void *pvfile = 0;
    void *pvfs = 0;
    err = vfs_mount(&pvfs, NULL, NULL);
    ASSERT(!err, "fii vfs mount : 0x%x\n", err)
    err = vfs_openbypath(pvfs, &pvfile, EXIF_CFG_PATH);
    if (err) {
        log_info("open %s, fail\n", EXIF_CFG_PATH);
        *addr = 0;
        *len = 0;
    } else {
        struct vfs_attr attr = {0};
        err = vfs_ioctl(pvfile, FS_IOCTL_FILE_ATTR, (int)&attr);
        ASSERT(!err, "fii vfs ioctl : 0x%x\n", err)
        *addr = attr.sclust;
        *len = attr.fsize;
        log_info(">>>exif addr:%x len:%x\n", *addr, *len);
    }
    if (pvfile) {
        vfs_file_close(&pvfile);
    }
    if (pvfs) {
        vfs_fs_close(&pvfs);
    }
}

static void vm_read_by_addr(u8 *buf, u32 addr, u32 len)
{
    sfc_norflash_read(NULL, buf, len, addr);
}

static u32 vm_write_by_addr(u8 *buf, u32 addr, u32 len)
{
    return sfc_norflash_write(NULL, buf, len, addr);
}

static u32 custom_cfg_item_write(u8 type, u8 *data, u16 data_len)
{
    ex_cfg_item_u item_u;
    u16 item_len = 0;
    ex_cfg_t *p_ex_cfg = NULL;
    u32 item_offset = 0;
    u32 exif_addr = exif_info.addr;

    u32 err = 0;

    if (type > sizeof(cfg_item_attr_tab) / sizeof(cfg_item_attr_tab[0])) {
        err = -2;
        goto _ERR_RET;
    }

    u32 item_data_len = cfg_item_attr_tab[type].data_len;

    if (item_data_len < data_len) {
        err = -1;
        goto _ERR_RET;
    }

    item_len = cfg_item_attr_tab[type].item_len;
    item_offset = cfg_item_attr_tab[type].member_offset;

    vm_read_by_addr((u8 *)&item_u.common_cfg, exif_addr + item_offset, item_len);
    if ((0xffff != item_u.common_cfg.crc) && \
        (0xffff != item_u.common_cfg.len)) {
        err = -3;
        goto _ERR_RET;
    }
    memset((u8 *)&item_u.common_cfg.data, 0x00, item_data_len);
    memcpy((u8 *)&item_u.common_cfg.data, data, data_len);
    item_u.common_cfg.len = data_len;
    item_u.common_cfg.crc = CRC16((u8 *)&item_u.common_cfg.data, data_len);

    vm_write_by_addr((u8 *)&item_u.common_cfg, exif_addr + item_offset, item_len);

_ERR_RET:
    log_error(">>>write item:%x err:%x\n", type, err);
    return err;
}

static u32 get_cfg_data_connect_by_name(u8 *name, u8 *data, u16 len, u16 *real_len)
{
    cfg_item_head_t cfg_item_head;
    cfg_head_t cfg_head;
    u32 err = CFG_ERR_NONE;
    u32 r_len;
    u16 i;
    *real_len = 0;
    u32 offset = 0;

    void *pvfile = 0;
    void *pvfs = 0;
    err = vfs_mount(&pvfs, NULL, NULL);
    ASSERT(!err, "fii vfs mount : 0x%x\n", err)
    err = vfs_openbypath(pvfs, &pvfile, RES_CUSTOM_CFG_FILE);
    if (err) {
        log_error("file open fail\n");
        err = CFG_ERR_FILE_READ_ERR;
        goto _ERR_RET;
    }

    r_len = fread(pvfile, (void *)&cfg_head, sizeof(cfg_head_t));
    if (r_len != sizeof(cfg_head_t)) {
        log_error("file read cfg head fail\n");
        err = CFG_ERR_FILE_READ_ERR;
        goto _ERR_RET;
    }
    offset += sizeof(cfg_head_t);
    log_info("read file req_len:%x ret_len:%x\n", sizeof(cfg_head_t), cfg_head.len);
    log_info("cfg_head:\n");
    log_info_hexdump((u8 *)&cfg_head, sizeof(cfg_head_t));
    for (i = 0; i < cfg_head.item_count; i++) {
        fseek(pvfile, SEEK_SET, offset);
        r_len = fread(pvfile, (void *)&cfg_item_head, sizeof(cfg_item_head_t));
        if (r_len != sizeof(cfg_item_head_t)) {
            log_error("file read cfg item head fail\n");
            err = CFG_ERR_FILE_READ_ERR;
            goto _ERR_RET;
        }
        offset += sizeof(cfg_item_head_t);
        if (0 == memcmp(cfg_item_head.name, name, strlen((const char *)name))) {
            log_info("find item %d name:%s\n", i, cfg_item_head.name);
            log_info_hexdump((u8 *)&cfg_item_head, sizeof(cfg_item_head_t));
            offset = cfg_item_head.addr;
            if (len < cfg_item_head.len) {
                err = CFG_ERR_ITEM_LEN_OVER;
                break;
            }
            *real_len = cfg_item_head.len;
            fseek(pvfile, offset, SEEK_SET);
            r_len = fread(pvfile, data, cfg_item_head.len);
            if (r_len != cfg_item_head.len) {
                log_error("read cfg item content fail\n");
                err = CFG_ERR_FILE_READ_ERR;
                goto _ERR_RET;
            }
            break;
        }
    }
    if (i == cfg_head.item_count) {
        err = CFG_ERR_ITEM_NO_FOUND;
    }
_ERR_RET:
    if (pvfile) {
        vfs_file_close(&pvfile);
    }
    if (pvfs) {
        vfs_fs_close(&pvfs);
    }
    return err;
}

static u32 custom_cfg_file_fill(ex_cfg_t *user_ex_cfg, u8 *write_flag)
{
    u32 err;
    u16 i;
    u8 *item_data;
    u8 *item_name;
    u16 item_len;
    for (i = 0 ; i < sizeof(cfg_item_description) / sizeof(cfg_item_description[0]); i++) {
        err = get_cfg_data_connect_by_name(cfg_item_description[i].item_name, \
                                           cfg_item_description[i].item_data, \
                                           cfg_item_description[i].item_len, \
                                           cfg_item_description[i].real_len);
        if (CFG_ERR_NONE != err) {
            log_error("cfg_item:%d read error:%x\n", i, err);
        } else {
            log_info("cfg_name %s\n cfg_len%x\n", cfg_item_description[i].item_name, *(cfg_item_description[i].real_len));
            log_info_hexdump((u8 *)cfg_item_description[i].item_data, *(cfg_item_description[i].real_len));
        }
        if (cfg_item_description[i].real_len != 0) {
            item_name = cfg_item_description[i].item_name;
            item_data = cfg_item_description[i].item_data;
            item_len = *cfg_item_description[i].real_len;
            if (0 == strcmp((const char *)item_name, "ver_info")) {
                custom_cfg_item_write(CFG_ITEM_VER_INFO, item_data, item_len);
            } else if (0 == strcmp((const char *)item_name, "reset_io")) {
                custom_cfg_item_write(CFG_ITEM_RESET_IO_INFO, item_data, item_len);
            } else if (0 == strcmp((const char *)item_name, "pilot_lamp_io")) {
                custom_cfg_item_write(CFG_ITEM_PILOT_LAMP_IO_INFO, item_data, item_len);
            } else if (0 == strcmp((const char *)item_name, "link_key")) {
                custom_cfg_item_write(CFG_ITEM_LINK_KEY_INFO, item_data, item_len);
            } else if (0 == strcmp((const char *)item_name, "power_io")) {
                custom_cfg_item_write(CFG_ITEM_POWER_IO_ON_OFF, item_data, item_len);
            } else if (0 == strcmp((const char *)item_name, "ver_info_ext")) {
#if VER_INFO_EXT_COUNT
                custom_cfg_item_write(CFG_ITEM_VER_INFO, item_data, item_len);
#endif
            }
        }
    }
    return err;
}

static u32 ex_cfg_fill_content(ex_cfg_t *user_ex_cfg, u8 *write_flag)
{
    custom_cfg_file_fill(NULL, write_flag);
    //CFG_ITEM_BT_NAME
    u8 *host_name = (u8 *)bt_get_local_name();
    u16 host_name_len = strlen(bt_get_local_name());
    custom_cfg_item_write(CFG_ITEM_BT_NAME, host_name, host_name_len);

    //CFG_ITEM_EDR_ADDR
    u8 addr[6];
    //hook_get_mac_addr(addr);
    custom_cfg_item_write(CFG_ITEM_EDR_ADDR, bt_get_mac_addr(), sizeof(addr));

    //CFG_ITEM_PIN_CODE
#if (0 == BT_CONNECTION_VERIFY)
    u8 pin_code[] = {BT_CONNECTION_VERIFY, VER_INFO_EXT_COUNT, 0, 0};
    u16 pin_code_len = sizeof(pin_code);
#else
    u8 *pin_code = (u8 *)bt_get_pin_code();
    u16 pin_code_len = strlen(bt_get_pin_code());
#endif
    custom_cfg_item_write(CFG_ITEM_PIN_CODE, pin_code, pin_code_len);


    //CFG_ITEM_BLE_NAME
    host_name = bt_get_local_name();
    host_name_len = strlen(bt_get_local_name());
    custom_cfg_item_write(CFG_ITEM_BLE_NAME, host_name, host_name_len);

    //CFG_ITEM_BLE_ADDR
    le_controller_get_mac(addr);
    //CFG_ITEM_SCAN_RSP
    u16 len;
    u8 *item_data = ble_get_scan_rsp_ptr(&len);
    log_info("get item_data\n");
    log_info_hexdump(item_data, len);
    struct excfg_rsp_payload rsp_payload;

    //New Scan_rsp
    /*
     *  |      len(1 Byte)     |     type(1 Byte)     |     data(name_len)    |
     *        bt_name_len      |         0x9          |        name_str       |
     *  jl_payloader_len(14)   |         0xff         |      jl_payloader     |
    */
    if (get_rcsp_support_new_reconn_flag()) {
        /* u8 *rsp_data = malloc(31); */
        u8 rsp_data[31] = {0};
        u8 i, rsp_len = 0;
        if (rsp_data) {
            log_info("[make new adv data]\n");
            rsp_payload.vid = 0x05D6;
            memcpy(rsp_payload.logo, "JLOTA", sizeof("JLOTA"));
            for (i = 0; i < sizeof(rsp_payload.logo) / 2; i++) {
                rsp_payload.logo[i] ^= rsp_payload.logo[sizeof(rsp_payload.logo) - i - 1];
                rsp_payload.logo[sizeof(rsp_payload.logo) - i - 1] ^= rsp_payload.logo[i];
                rsp_payload.logo[i] ^= rsp_payload.logo[sizeof(rsp_payload.logo) - i - 1];
            }
            rsp_payload.version = 0;
            memcpy(rsp_payload.addr, addr, 6);

            i = 0;
            item_data = ble_get_adv_data_ptr(&len);
            while (item_data[i] && i < len) {
                if (*(item_data + 1) != 0xff && *(item_data + 1) != 0x09) {
                    memcpy(rsp_data + i, item_data, *item_data + 1);
                    rsp_len += *item_data + 1;
                }
                i += (1 + *item_data);
                item_data += (1 + *item_data);
            }

            if (rsp_len + sizeof(struct excfg_rsp_payload) + 2 > 31) {
                log_error("adv data overflow!!!\n");
            } else {
                *(rsp_data + rsp_len) = sizeof(struct excfg_rsp_payload) + 1;        //fill jlpayload
                *(rsp_data + rsp_len + 1) = 0xff;                                    // HCI_EIR_DATATYPE_MANUFACTURER_SPECIFIC_DATA
                memcpy(rsp_data + rsp_len + 2, &rsp_payload, sizeof(struct excfg_rsp_payload));
                rsp_len += (2 + sizeof(struct excfg_rsp_payload));
                addr[0] += 1;                                                        //修改地址，让手机重新发现服务, 这里地址的修改规则可以用户自行设置
                log_info("new adv_data:\n");
                log_info_hexdump(rsp_data, rsp_len);
                custom_cfg_item_write(CFG_ITEM_ADV_IND, rsp_data, rsp_len);
            }

            //广播包里有0xff字段也要找出来去掉，小程序判断到adv和rsp有重复字段是会出错
            u8 new_adv_len = 0;
            memset(rsp_data, 0, sizeof(rsp_data));
            /* item_data = ble_get_adv_data_ptr(&len); */
#if 1
            u8 *name_p = bt_get_local_name();
            new_adv_len += make_eir_packet_data(rsp_data, 0, HCI_EIR_DATATYPE_COMPLETE_LOCAL_NAME, (void *)name_p, strlen(name_p));
#else
            i = 0;
            u8 *item_data = ble_get_scan_rsp_ptr(&len);
            while (i < len) {                           //找出不等于0xff的信息,拷贝到new_adv_data
                /* if (*(item_data + 1) != 0xff) { */
                if (*(item_data + 1) == 0x09) {
                    //memcpy(rsp_data, item_data, *item_data + 1);
                    memcpy(rsp_data + i, item_data, *item_data + 1);
                    new_adv_len += *item_data + 1;
                }
                i += (1 + *item_data);
                item_data += (1 + *item_data);
            }
#endif
            log_info("new rsp_data:\n");
            log_info_hexdump(rsp_data, new_adv_len);
            custom_cfg_item_write(CFG_ITEM_SCAN_RSP, rsp_data, new_adv_len);

            /* free(rsp_data); */
        }
    } else {
        custom_cfg_item_write(CFG_ITEM_SCAN_RSP, item_data, len);
        /* CFG_ITEM_ADV_IND */
        item_data = ble_get_adv_data_ptr(&len);
        custom_cfg_item_write(CFG_ITEM_ADV_IND, item_data, len);
    }
    custom_cfg_item_write(CFG_ITEM_BLE_ADDR, addr, sizeof(addr));

    // 暂时打开会死机，先屏蔽
    //CFG_ITEM_GATT_PROFILE
    /* item_data = ble_get_gatt_profile_data(&len); */
    /* custom_cfg_item_write(CFG_ITEM_GATT_PROFILE, item_data, len); */


    //CFG_ITEM_LAST_DEVICE_LINK_KEY_INFO
    /* item_data = get_last_device_connect_linkkey(&len); */
    /* put_buf(item_data, len); */
    /* custom_cfg_item_write(CFG_ITEM_LAST_DEVICE_LINK_KEY_INFO, item_data, len); */

#if VER_INFO_EXT_COUNT
    u8 authkey_len = 0;
    u8 *local_authkey_data = NULL;
    get_authkey_procode_from_cfg_file(&local_authkey_data, &authkey_len, GET_AUTH_KEY_FROM_EX_CFG);
    custom_cfg_item_write(CFG_ITEM_VER_INFO_AUTHKEY, local_authkey_data, authkey_len);

    u8 procode_len = 0;
    u8 *local_procode_data = NULL;
    get_authkey_procode_from_cfg_file(&local_procode_data, &procode_len, GET_PRO_CODE_FROM_EX_CFG);

    custom_cfg_item_write(CFG_ITEM_VER_INFO_PROCODE, local_procode_data, procode_len);
#endif

    u16 pvid[2] = {0};
    pvid[0] =  get_vid_pid_ver_from_cfg_file(GET_VID_FROM_EX_CFG);
    pvid[1] =  get_vid_pid_ver_from_cfg_file(GET_PID_FROM_EX_CFG);
    custom_cfg_item_write(CFG_ITEM_PVID, pvid, sizeof(pvid));


    u32 err = 0;
    void *pvfile = 0;
    void *pvfs = 0;
    u8 md5[32] = {0};
    err = vfs_mount(&pvfs, NULL, NULL);
    if (0 == err) {
        err = vfs_openbypath(pvfs, &pvfile, "/md5.bin");
        if (0 == err) {
            fread(pvfile, (void *)md5, 32);
        }
    }
    if (pvfile) {
        vfs_file_close(&pvfile);
    }
    if (pvfs) {
        vfs_fs_close(&pvfs);
    }
    custom_cfg_item_write(CFG_ITEM_MD5, md5, sizeof(md5));


    u8 sdk_type = RCSP_SDK_TYPE;
    custom_cfg_item_write(CFG_ITEM_SDK_TYPE, &sdk_type, sizeof(sdk_type));

    return 0;
}

u32 ex_cfg_fill_content_api(void)
{
    ex_cfg_get_addr_and_len(&exif_info.addr, &exif_info.len);
    if (exif_info.addr) {
#if (EXIF_ERASE_CONFIG == ALWAYS_ERASE_EXIF_AREA)
        if (exif_info.len < SECTOR_ERASER) {
            ASSERT(0, "exif len less SECOTR_SiZE");
        }
        norflash_erase(IOCTL_ERASE_SECTOR, exif_info.addr);
#endif
        ex_cfg_fill_content(NULL, NULL);
    }
    return exif_info.addr;
}

u16 get_vid_pid_ver_from_cfg_file(u8 type)
{
    u8 *item_name = cfg_item_description[0].item_name;
    u8 *item_data = cfg_item_description[0].item_data;
    u16 item_len = cfg_item_description[0].item_len;
    u16 *real_len = cfg_item_description[0].real_len;
    if (!(*real_len)) {
        get_cfg_data_connect_by_name(item_name, item_data, item_len, real_len);
    }
    switch (type) {
    case GET_VID_FROM_EX_CFG:
        return ((u16)item_data[0] << 8 | (u16)item_data[1]);
    case GET_PID_FROM_EX_CFG:
        return ((u16)item_data[2] << 8 | (u16)item_data[3]);
    case GET_VER_FROM_EX_CFG:
        return ((u16)item_data[4] << 8 | (u16)item_data[5]);
    }

    return ((u16) - 1);
}

#if VER_INFO_EXT_COUNT
u32 get_authkey_procode_from_cfg_file(u8 *data[], u8 *len, u8 type)
{
    u8 *item_name = cfg_item_description[5].item_name;
    u8 *item_data = cfg_item_description[5].item_data;
    u16 item_len = cfg_item_description[5].item_len;
    u16 *real_len = cfg_item_description[5].real_len;
    if (!(*real_len)) {
        get_cfg_data_connect_by_name(item_name, item_data, item_len, real_len);
    }
    u8 offset = (u8) - 1;
    u8 index = 0;
    u8 separator = ',';
    while (item_data[++offset]) {
        if (separator == item_data[offset]) {
            if (type - GET_AUTH_KEY_FROM_EX_CFG) {
                index = offset;
                type --;
            } else {
                break;
            }
        }
    }
    if (!item_data[offset]) {
        index ++;
    }
    *data = item_data + index;
    *len = offset - index;
    return 0;
}
#endif

#endif
