/* #include "flash_wp.h" */
/* #include "flash_init.h" */
/* #include "tick_timer_driver.h" */
#include "device.h"
#include "vfs.h"
#include "msg.h"
#include "sys_memory.h"
#include "vm_sfc.h"
#include "my_malloc.h"
#include "boot.h"
#include "app_config.h"
#include "bt_config_tool.h"
#include "crc16.h"
#include "reserved_area/reserved_area.h"

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[cfg_bin]"
#include "log.h"


static u8 reserve_bt_mac[6];
static struct vfs_attr btif_attr;
extern void swapX(const uint8_t *src, uint8_t *dst, int len);
extern int sfc_norflash_read(struct device *device, void *buf, u32 len, u32 offset);

/*****************mac+crc*******************/
/*
 *start addr:reserved area last 256 byte
  data start:start addr + 64 byte
 * */

static u8 *get_btif_mac_data(void)
{
    u8 read_buf[8];
    void *device = 0;

    u32 cfg_reserve = boot_info.flash_size - RESERVED_MAC_ADDR_OFFSET;
    sfc_norflash_read(device, read_buf, 8, cfg_reserve);
    swapX(read_buf, reserve_bt_mac, 6);
    /* log_info("cfg addr: 0x%x >>> ", cfg_reserve); */
    /* log_info_hexdump((u8*)reserve_bt_mac ,6); */
    return reserve_bt_mac;
}

static bool check_btif_mac_area(u8 *addr_info)
{
    //检查mac地址是否为全0 或 全FF
    const u8 mac_buf_tmp[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    const u8 mac_buf_tmp2[6] = {0, 0, 0, 0, 0, 0};

    if (!memcmp(addr_info, mac_buf_tmp, 6)) {
        return false;
    }

    if (!memcmp(addr_info, mac_buf_tmp2, 6)) {
        return false;
    }
    return true;
}


int cfg_bin_init(void)
{
    u32 err;
    void *pvfs = 0;
    void *pvfile = 0;

    err = vfs_mount(&pvfs, (void *)NULL, (void *) NULL);
    if (err) {
        log_error("cfg_bin_init mount fail 0x%x\n", err);
        return 0;
    }
    err = vfs_openbypath(pvfs, &pvfile, SYSCFG_DEFAULT_BIN_PATH);
    if (err) {
        log_error("cfg_bin_init openpath fail 0x%x\n", err);
        return 0;
    }
    err = vfs_get_attrs(pvfile, &btif_attr);
    if (err) {
        log_error("cfg_bin_init get_attrs fail 0x%x\n", err);
        return 0;
    }
    /* log_info("cfg_tool size : 0x%x\ncfg_tool sclust : 0x%x\n", btif_attr.fsize, btif_attr.sclust); */

    vfs_file_close(&pvfile);
    vfs_fs_close(&pvfs);

    //获取-res中cfg_tool.bin的cpu地址(cfg_tool.bin地址 + app地址)
    u32 addr = btif_attr.sclust + boot_info.sfc.app_addr;
    /* log_info("addr: 0x%x >>> ", addr); */
    /* log_info_hexdump((u8*)addr ,btif_attr.fsize); */

    //获取预留区中的烧录的mac地址
    u8 reserved_ret = check_btif_mac_area(get_btif_mac_data());

    rsv_auth_analysis();

    //找到cfg_tool.bin中BT对应的ble_name、mac_addr、rf_power
    u8 ble_name[32], mac_addr[6], rf_power;
    memcpy(ble_name, (void *)(addr + CFGBIN_BLE_NAME_OFFSET), 32);
    memcpy(mac_addr, (void *)(addr + CFGBIN_MAC_ADDR_OFFSET), 6);
    memcpy(&rf_power, (void *)(addr + CFGBIN_RF_POWER_OFFSET), 1);
    //配置文件放进VM中
    sysmem_write_api(CFG_BT_NAME, ble_name, 32);
    sysmem_write_api(CFG_BT_RF_POWER_ID, &rf_power, 1);

    //read ble_name、rf_power demo
    memset(ble_name, 0xaa, 32);
    rf_power = 0xaa;

    sysmem_read_api(CFG_BT_NAME, ble_name, 32);
    /* log_info("vm read ble_name %s\n",ble_name); */
    /* log_info_hexdump((u8*)ble_name ,32); */

    sysmem_read_api(CFG_BT_RF_POWER_ID, &rf_power, 1);
    /* log_info("vm read rf_power : %d\n", rf_power); */

    if (reserved_ret) {
        sysmem_write_api(CFG_BT_MAC_ADDR, get_btif_mac_data(), 6);
    } else {
        u8 read_mac_bin = check_btif_mac_area(mac_addr);
        if (read_mac_bin) {
            sysmem_write_api(CFG_BT_MAC_ADDR, mac_addr, 6);
        }
    }

    //read mac_addr demo
    memset(mac_addr, 0x00, 6);

    sysmem_read_api(CFG_BT_MAC_ADDR, mac_addr, 6);
    /* log_info("vm read mac_addr\n"); */
    /* log_info_hexdump((u8*)mac_addr ,6); */

    return 0;
}



