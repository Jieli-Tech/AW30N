// *INDENT-OFF*
#include "app_config.h"
#include  "maskrom_stubs.ld"
#include "irq_vec.h"

#define _ADDR_RAM0_END    0x3f10000
#define _ADDR_RAM0_START  (0x3f00000 + MAX_IRQ_ENTRY_NUM * 4 + 0x200)
#define _ADDR_CACHE_RAM_START 0x3f30000

UPDATA_SIZE     = 0x80;
UPDATA_BEG      = _ADDR_RAM0_END - UPDATA_SIZE;
ICACHE_RAM_SIZE = 0x4000;

//config
MEMORY
{
    // app_code: LENGTH = 32M-0x100, 超过32M运行代码需要使用长跳转;
    app_code(rx)        : ORIGIN = _SFC_MEMORY_START_ADDR + 0x100,  LENGTH = 32M-0x100
    irq_vec(rx)         : ORIGIN = _IRQ_MEM_ADDR,                   LENGTH = MAX_IRQ_ENTRY_NUM * 4
    ram0(rw)            : ORIGIN = _ADDR_RAM0_START,                LENGTH = _ADDR_RAM0_END - _ADDR_RAM0_START - 0x24
    boot_ram(rw)        : ORIGIN = _ADDR_RAM0_END - 0x24,           LENGTH = 0x24
}
ENTRY(_start)

SECTIONS
{

	. = ORIGIN(boot_ram);
	.boot_data ALIGN(4):
	{
		*(.boot_info)
	} > boot_ram

    /* L1 memory sections */
    . = ORIGIN(ram0);
    .data ALIGN(4):
    {
        PROVIDE(data_buf_start = .);
        *(.data_magic)
        *(.data)
        *(.*.data)
        *(.common)
        *(.ble_app_data)

		cache_Lx_code_text_begin = .;
        *iumoddi3.o(.text .rodata*)
        *(.audio_isr_text)
        *(.*.text.cache.L1)
        *(.*.text.cache.L2)
        *(.*.text.cache.L3)
        *(.log_ut_text)
        . = (. + 3) / 4 * 4 ;
        cache_Lx_code_text_end = .;

        #include "bt_include/btstack_lib_data.ld"
        #include "bt_controller_include/btctler_lib_data.ld"

    } > ram0

	.debug_data ALIGN(4):
	{
        PROVIDE(debug_buf_start = .);
        *(.debug.data.bss)
        *(.debug.data)
        . = (. + 3) / 4 * 4 ;
    } > ram0

    .bss (NOLOAD) : SUBALIGN(4)
    {
        PROVIDE(bss_buf_start = .);
        . = ALIGN(32);
        _cpu0_sstack_begin = .;
        PROVIDE(cpu0_sstack_begin = .);
        . = ALIGN(32);
        *(.intr_stack)
        . = ALIGN(32);
		*(.stack_magic);
        . = ALIGN(32);
#if RUN_APP_DONGLE
        . += 0xc00;
#endif
        *(.stack)
        . = ALIGN(32);
		*(.stack_magic0);
        . = ALIGN(32);
        _cpu0_sstack_end = .;
        PROVIDE(cpu0_sstack_end = .);
        . = ALIGN(32);
		_system_data_begin = .;
        *(.bss)
        *(.*.data.bss)
        *(.non_volatile_ram)
        *(.msd.keep_ram)
        *(.usb_hid.keep_ram)
        *(.ble_app_bss)

        #include "bt_include/btstack_lib_bss.ld"
        #include "bt_controller_include/btctler_lib_bss.ld"

        _system_data_end = .;
    } > ram0

    .nv_ram_malloc ALIGN(4)://for bt
    {
        PROVIDE(_nv_ram_malloc_start = .);
        KEEP(*(.sec_bt_nv_ram))
        PROVIDE(_nv_ram_malloc_end = .);
    } > ram0

#if RUN_APP_CUSTOM
#if FULL_DUPLEX_RADIO
    #include "../post_build/bd49/mbox_flash/app_ld_overlay_fullduplex_radio.c"
#else
    #include "../post_build/bd49/mbox_flash/app_ld_overlay_custom.c"
#endif

#elif RUN_APP_RC
    #include "../post_build/bd49/mbox_flash/app_ld_overlay_rc.c"
#elif RUN_APP_DONGLE
    #include "../post_build/bd49/mbox_flash/app_ld_overlay_dongle.c"
#else
    #error "LD NO OVERLAY SEC"
#endif

    /* d_dec_max = MAX(mp3_st_buf_end, wav_buf_end); */

     .heap_buf ALIGN(4):
    {
        PROVIDE(_free_start = .);
        . = LENGTH(ram0) + ORIGIN(ram0) - 1;
        PROVIDE(_free_end = .);
    } > ram0

    _ram_end = .;


    . = ORIGIN(app_code);
    .app_code ALIGN(32):
    {

        app_code_text_begin = .;
        KEEP(*(.chip_entry_text))
        *(.start.text)
        *(.*.text.const)
        *(.*.text)
        *(.version)
        *(.debug)
        *(.debug.text.const)
        *(.debug.text)
        *(.debug.string)
        *(.text)
        _SPI_CODE_START = .;
        *(.vm_sfc.text.cache)
        . = ALIGN(4);
        _SPI_CODE_END = .;
        *(.rodata*)
        *(.ins)
        app_code_text_end = . ;

        . = ALIGN(4);
        vfs_ops_begin = .;
        KEEP(*(.vfs_operations))
        vfs_ops_end = .;

        . = ALIGN(4);
        app_main_begin = .;
        KEEP(*(.app_main))
        app_main_end = .;

        . = ALIGN(4);
        PROVIDE(device_node_begin = .);
        KEEP(*(.device))
        PROVIDE(device_node_end = .);

        . = ALIGN(4);
        hsb_critical_handler_begin = .;
        KEEP(*(.hsb_critical_txt))
        hsb_critical_handler_end = .;

        . = ALIGN(4);
        lsb_critical_handler_begin = .;
        KEEP(*(.lsb_critical_txt))
        lsb_critical_handler_end = .;

        . = ALIGN(4);
        loop_detect_handler_begin = .;
        KEEP(*(.loop_detect_region))
        loop_detect_handler_end = .;

		. = ALIGN(4);
		lp_target_begin = .;
	    PROVIDE(lp_target_begin = .);
	    KEEP(*(.lp_target))
	    lp_target_end = .;
	    PROVIDE(lp_target_end = .);

		. = ALIGN(4);
	    lp_request_begin = .;
	    PROVIDE(lp_request_begin = .);
	    KEEP(*(.lp_request))
	    lp_request_end = .;
	    PROVIDE(lp_request_end = .);

		. = ALIGN(4);
        deepsleep_target_begin = .;
        PROVIDE(deepsleep_target_begin = .);
        KEEP(*(.deepsleep_target))
        deepsleep_target_end = .;
        PROVIDE(deepsleep_target_end = .);

		. = ALIGN(4);
		p2m_msg_handler_begin = .;
		PROVIDE(p2m_msg_handler_begin = .);
		KEEP(*(.p2m_msg_handler))
		PROVIDE(p2m_msg_handler_end = .);
		p2m_msg_handler_end = .;

		. = ALIGN(4);
		phw_begin = .;
		PROVIDE(phw_begin = .);
		KEEP(*(.phw_operation))
		PROVIDE(phw_end = .);
		phw_end = .;



		. = ALIGN(4);
        bt_app_start = .;
		PROVIDE(bt_app_begin = .);
		KEEP(*(.ble_app_text))
		KEEP(*(.ble_app_text_const))
		PROVIDE(bt_app_end = .);
        bt_app_end = .;

		. = ALIGN(4);
        #include "bt_include/btstack_lib_text.ld"
		. = ALIGN(4);
        #include "bt_controller_include/btctler_lib_text.ld"

        . = ALIGN(32);
        /* . = LENGTH(app_code) - SIZEOF(.data); */
        /* text_end = .; */
	} >app_code


}

#include "update/code_v2/update.ld"

//================== Section Info Export ====================//
bss_begin       = ADDR(.bss);
bss_size        = SIZEOF(.bss);

/*除堆栈外的bss区*/
bss_size1       = _system_data_end - _system_data_begin;
bss_begin1      = _system_data_begin;

text_begin      = ADDR(.app_code);
text_size       = SIZEOF(.app_code);
text_end        = text_begin + text_size;

data_addr  = ADDR(.data) ;
data_begin = text_end ;
data_size =  SIZEOF(.data) + SIZEOF(.debug_data);


/* overlay_a_size = __overlay_a_end - __overlay_a_start; */
/* overlay_b_size = __overlay_b_end - __overlay_b_start; */
/* music_play_size = mode_music_overlay_data_end - mode_music_overlay_data_start; */
/* simple_decode_size = mode_smpl_dec_ovly_end - mode_smpl_dec_ovly_start; */
/* loud_speaker_size = mode_loud_spk_overlay_data_end - mode_loud_spk_overlay_data_start; */
/* linein_size = mode_linein_overlay_data_end - mode_linein_overlay_data_start; */
/* usb_slave_size = mode_pc_overlay_data_end - mode_pc_overlay_data_start; */
/* ar_trans_size = ar_trans_data_end - ar_trans_data_start; */
/* rf_radio_size = rf_radio_data_end - rf_radio_data_start; */
/* opus_enc_size = enc_opus_data_end - rec_data_start; */
/* mp3_enc_size = enc_mp3_data_end - rec_data_start; */
bt_app_size = bt_app_end - bt_app_start;
heap_size = _free_end - _free_start;
