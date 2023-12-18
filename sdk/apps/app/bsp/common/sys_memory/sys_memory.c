#include "sys_memory.h"
#include "app_config.h"
#include "cpu.h"

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[SYS_MEM]"
#include "log.h"


#if (SYS_MEMORY_SELECT == USE_NEW_VM)
/* 新版vm，支持预擦除 */
#include "new_vm.h"
#define sysmem_phy_init(addr, size)         nvm_init_api(addr, size)
#define sysmem_phy_read(id, buf, len)       nvm_read_api(id, buf, len)
#define sysmem_phy_write(id, buf, len)      nvm_write_api(id, buf, len)
#define sysmem_phy_pre_erase()              nvm_erasure_next_api()
#elif (SYS_MEMORY_SELECT == USE_OLD_VM)
/* 旧版vm */
#include "vm.h"
#define sysmem_phy_init(addr, size)         syscfg_vm_init_phy(addr, size)
#define sysmem_phy_read(id, buf, len)       vm_read_phy(id, buf, len)
#define sysmem_phy_write(id, buf, len)      vm_write_phy(id, buf, len)
#define sysmem_phy_pre_erase(...)
#else
#define sysmem_phy_init(addr, size)         -1
#define sysmem_phy_read(id, buf, len)       -1
#define sysmem_phy_write(id, buf, len)      -1
#define sysmem_phy_pre_erase(...)
#endif


int sysmem_init_api(u32 mem_addr, u32 mem_size)
{
    return sysmem_phy_init(mem_addr, mem_size);
}

int sysmem_read_api(u32 id, u8 *data_buf, u16 len)
{
    return sysmem_phy_read(id, data_buf, len);
}

int sysmem_write_api(u32 id, u8 *data_buf, u16 len)
{
    return sysmem_phy_write(id, data_buf, len);
}

void sysmem_pre_erase_api(void)
{
    sysmem_phy_pre_erase();
}

// 蓝牙存储使用接口
int syscfg_write(u16 item_id, const void *buf, u16 len)
{
    return sysmem_write_api(item_id, (u8 *)buf, len);
}

int syscfg_read(u16 item_id, void *buf, u16 len)
{
    return sysmem_read_api(item_id, buf, len);
}
