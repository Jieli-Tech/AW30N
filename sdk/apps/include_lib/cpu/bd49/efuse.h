#ifndef  __EFUSE_H__
#define  __EFUSE_H__

#include "cpu.h"

// bool is_boot_otp();

void efuse_init();
void efuse_dump();

u8 efuse_get_efuse_en_act();
u8 efuse_get_lvd_rst_en();
u8 efuse_get_vddio_lvd_lev();
u8 efuse_get_lvd_vbg_lel();
u8 efuse_get_vddio_lev();
u8 efuse_get_mclr_en();
u8 efuse_get_fast_up();
u8 efuse_get_pinr_reset_en();
u8 efuse_get_audio_lvbg_trim();
u8 efuse_get_audio_hvbg_trim();
u8 efuse_get_sfc_fast_boot_dis();
u8 efuse_get_trim_act();
u8 efuse_get_mvbg_lev();
u8 efuse_get_wvbg_level_trim();

u32 efuse_get_chip_id();
u32 get_chip_id();
u32 get_chip_version();
u8 efuse_get_cp_pass();
u8 efuse_get_ft_pass();
u8 efuse_get_wvdd_level();

u32 efuse_get_lrc_pll_nr();



#endif  /* EFUSE_H */
