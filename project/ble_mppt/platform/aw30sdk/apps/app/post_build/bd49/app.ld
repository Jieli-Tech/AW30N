










  __local_irq_enable = ABSOLUTE(0x3f43eb0);
  __local_irq_disable = ABSOLUTE(0x3f43eb4);
  request_irq = ABSOLUTE(0x3f43eb8);
  p33_buf = ABSOLUTE(0x3f43ebc);
  p33_xor_1byte = ABSOLUTE(0x3f43ec0);
  p33_and_1byte = ABSOLUTE(0x3f43ec4);
  p33_or_1byte = ABSOLUTE(0x3f43ec8);
  p33_tx_1byte = ABSOLUTE(0x3f43ecc);
  p33_rx_1byte = ABSOLUTE(0x3f43ed0);
  p33_rx_nbyte = ABSOLUTE(0x3f43ed4);
  P33_CON_SET = ABSOLUTE(0x3f43ed8);
  memcpy = ABSOLUTE(0x3f43edc);
  memmove = ABSOLUTE(0x3f43ee0);
  memcmp = ABSOLUTE(0x3f43ee4);
  memset = ABSOLUTE(0x3f43ee8);
  strcmp = ABSOLUTE(0x3f43eec);
  strcpy = ABSOLUTE(0x3f43ef0);
  strlen = ABSOLUTE(0x3f43ef4);
  strncmp = ABSOLUTE(0x3f43ef8);
  strchr = ABSOLUTE(0x3f43efc);
  ic_pll_init = ABSOLUTE(0x3f43f00);
  sfc_suspend = ABSOLUTE(0x3f43f04);
  sfc_resume = ABSOLUTE(0x3f43f08);
  sfc_drop_cache = ABSOLUTE(0x3f43f0c);
  sfc_boot = ABSOLUTE(0x3f43f10);
  sfc_entry_continue_mode = ABSOLUTE(0x3f43f14);
  ota_jump_mode = ABSOLUTE(0x3f43f18);
  IcuCmdKst = ABSOLUTE(0x3f43f1c);
  IcuCmdSend = ABSOLUTE(0x3f43f20);
  IcuEnable = ABSOLUTE(0x3f43f24);
  IcuDisable = ABSOLUTE(0x3f43f28);
  IcuInitial = ABSOLUTE(0x3f43f2c);
  IcuLockRegion = ABSOLUTE(0x3f43f30);
  IcuFlushinvAll = ABSOLUTE(0x3f43f34);
  chip_crc16 = ABSOLUTE(0x3f43f38);
  CrcDecode = ABSOLUTE(0x3f43f3c);
  mask_init = ABSOLUTE(0x3f43f40);
  wdt_clear = ABSOLUTE(0x3f43f44);
  nvram_set_boot_state = ABSOLUTE(0x3f43f48);
  chip_reset = ABSOLUTE(0x3f43f4c);
  flash_poweron = ABSOLUTE(0x3f43f50);
  flash_poweron_base = ABSOLUTE(0x3f43f54);
  flash_poweroff = ABSOLUTE(0x3f43f58);
  flash_poweroff_base = ABSOLUTE(0x3f43f5c);
  spi_io_mount = ABSOLUTE(0x3f43f60);
  spi_io_unmount = ABSOLUTE(0x3f43f64);
  get_sfc_read_mode = ABSOLUTE(0x3f43f68);
  spi_port_io_init = ABSOLUTE(0x3f43f6c);
  spi_port_io_uninit = ABSOLUTE(0x3f43f70);
  usb_slave_mode = ABSOLUTE(0x3f43f74);
  doe = ABSOLUTE(0x3f43f78);
  LZ4_setStreamDecode = ABSOLUTE(0x3f43f7c);
  LZ4_decompress_safe_continue = ABSOLUTE(0x3f43f80);
  LZ4_decompress_generic = ABSOLUTE(0x3f43f84);
  LZ4_decompress_safe_usingDict = ABSOLUTE(0x3f43f88);
  dec_isd_cfg_ini = ABSOLUTE(0x3f43f8c);
  cbuf_read = ABSOLUTE(0x3f43f90);
  cbuf_write = ABSOLUTE(0x3f43f94);
  cbuf_is_write_able = ABSOLUTE(0x3f43f98);
  cbuf_write_alloc = ABSOLUTE(0x3f43f9c);
  cbuf_write_updata = ABSOLUTE(0x3f43fa0);
  cbuf_read_alloc = ABSOLUTE(0x3f43fa4);
  cbuf_read_updata = ABSOLUTE(0x3f43fa8);
  cbuf_init = ABSOLUTE(0x3f43fac);
  cbuf_clear = ABSOLUTE(0x3f43fb0);
  cbuf_rewrite = ABSOLUTE(0x3f43fb4);
  tab_read = ABSOLUTE(0x3f43fb8);
  lbuf_init = ABSOLUTE(0x3f43fbc);
  lbuf_avaliable = ABSOLUTE(0x3f43fc0);
  lbuf_state = ABSOLUTE(0x3f43fc4);
  lbuf_dump = ABSOLUTE(0x3f43fc8);
  lbuf_free_space = ABSOLUTE(0x3f43fcc);
  lbuf_alloc = ABSOLUTE(0x3f43fd0);
  lbuf_remain_space = ABSOLUTE(0x3f43fd4);
  lbuf_realloc = ABSOLUTE(0x3f43fd8);
  lbuf_empty = ABSOLUTE(0x3f43fdc);
  lbuf_clear = ABSOLUTE(0x3f43fe0);
  lbuf_real_size = ABSOLUTE(0x3f43fe4);
  lbuf_inc_ref = ABSOLUTE(0x3f43fe8);
  lbuf_push = ABSOLUTE(0x3f43fec);
  lbuf_traversal = ABSOLUTE(0x3f43ff0);
  lbuf_pop = ABSOLUTE(0x3f43ff4);
  lbuf_free_check = ABSOLUTE(0x3f43ff8);
  lbuf_free = ABSOLUTE(0x3f43ffc);
  _IRQ_MEM_ADDR = ABSOLUTE(0x3f00000);
  _RAM_LIMIT_L = ABSOLUTE(0x3f00000);
  _MASK_USED_MEM_END = ABSOLUTE(0x3f00bc4);
  _MASK_EXPORT_MEM_BEGIN = ABSOLUTE(0x3f00100);
  _MASK_EXPORT_MEM_SIZE = ABSOLUTE(0xa4);
  _SFC_MEMORY_START_ADDR = ABSOLUTE(0x4000000);
  g_boot_soft_flag = ABSOLUTE(0x3f00100);
  efuse_page0 = ABSOLUTE(0x3f0017c);
  efuse_page1 = ABSOLUTE(0x3f00184);
  nvram_list = ABSOLUTE(0x3f0010c);
  exception_irq_handler = ABSOLUTE(0x3f4003a);







UPDATA_SIZE = 0x80;
UPDATA_BEG = 0x3f10000 - UPDATA_SIZE;
ICACHE_RAM_SIZE = 0x4000;


MEMORY
{




    app_code(rx) : ORIGIN = _SFC_MEMORY_START_ADDR + 0x100, LENGTH = 32M-0x100
    irq_vec(rx) : ORIGIN = _IRQ_MEM_ADDR, LENGTH = 64 * 4
    ram0(rw) : ORIGIN = (0x3f00000 + 64 * 4 + 0x200), LENGTH = 0x3f10000 - (0x3f00000 + 64 * 4 + 0x200) - 0x24
    boot_ram(rw) : ORIGIN = 0x3f10000 - 0x24, LENGTH = 0x24
}
ENTRY(_start)

SECTIONS
{
 . = ORIGIN(boot_ram);
 .boot_data ALIGN(4):
 {
  *(.boot_info)
 } > boot_ram


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



        btstack_data_start = .;
        *(.bt_stack_data)
        *(.ble_db_data)
        *(.ble_sm_data)
        *(.ble_att_data)
        *(.ble_gatt_data)


        BTSTACK_LE_HOST_MESH_DATA_START = .;
  . = (. +3) & ~ 3;
        _net_buf_pool_list = .;
        *(._net_buf_pool.static.*)

        *(.ble_mesh_data)
        *(.ble_mesh_tinycrypt_data)
        BTSTACK_LE_HOST_MESH_DATA_SIZE = ABSOLUTE(. - BTSTACK_LE_HOST_MESH_DATA_START);


        btstack_data_end = .;

        btctler_data_start = .;

        BTCTLER_CONTROLLER_DATA_START = .;
        *(.bt_rf_data)
  *(.vendor_manager_data)
  *(.hci_controller_data)
        BTCTLER_CONTROLLER_DATA_SIZE = ABSOLUTE(. - BTCTLER_CONTROLLER_DATA_START);

        BTCTLER_LE_CONTROLLER_DATA_START = .;
        *(.ble_ll_data)
        *(.ble_rf_data)
        *(.ble_hci_data)

        BTCTLER_LE_CONTROLLER_DATA_SIZE = ABSOLUTE(. - BTCTLER_LE_CONTROLLER_DATA_START);

        BTCTLER_CL_DATA_START = .;
        BTCTLER_CL_DATA_SIZE = ABSOLUTE(. - BTCTLER_CL_DATA_START);

        btctler_data_end = .;


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



        btstack_bss_start = .;
        *(.bt_stack_bss)
        *(.ble_db_bss)
        *(.ble_sm_bss)
        *(.ble_att_bss)
        *(.ble_gatt_bss)
        *(.btstack_pool)


        BTSTACK_LE_HOST_MESH_BSS_START = .;
        *(.ble_mesh_bss)
        *(.ble_mesh_tinycrypt_bss)
        BTSTACK_LE_HOST_MESH_BSS_SIZE = ABSOLUTE(. - BTSTACK_LE_HOST_MESH_BSS_START);

        btstack_bss_end = .;

        btctler_bss_start = .;

        BTCTLER_CONTROLLER_BSS_START = .;


        *(.bt_rf_data_bss)
  *(.hci_controller_data_bss)
  *(.vendor_manager_data_bss)
        BTCTLER_CONTROLLER_BSS_SIZE = ABSOLUTE(. - BTCTLER_CONTROLLER_BSS_START);

        BTCTLER_LE_CONTROLLER_BSS_START = .;
        *(.ble_hci_data_bss)
        *(.ble_ll_data_bss)
        *(.ble_rf_data_bss)
        BTCTLER_LE_CONTROLLER_BSS_SIZE = ABSOLUTE(. - BTCTLER_LE_CONTROLLER_BSS_START);

        BTCTLER_CL_BSS_START = .;
        BTCTLER_CL_BSS_SIZE = ABSOLUTE(. - BTCTLER_CL_BSS_START);

        btctler_bss_end = .;


        _system_data_end = .;
    } > ram0

    .nv_ram_malloc ALIGN(4):
    {
        PROVIDE(_nv_ram_malloc_start = .);
        KEEP(*(.sec_bt_nv_ram))
        PROVIDE(_nv_ram_malloc_end = .);
    } > ram0







    __overlay_a_start = .;
    OVERLAY :
    AT(0xA0000000) SUBALIGN(4)
    {
        .startup_data {
            . = ALIGN(4);
            PROVIDE(startup_data_start = .);
            *(.cache_way_setting_text)
            . = (. + 3) / 4 * 4;
            PROVIDE(startup_data_end = .);
        }



        .d_music_play {
            PROVIDE(mode_music_overlay_data_start = .);
            *(.mode_music_overlay_data);

            *(.fat_buf);
            PROVIDE(mode_music_overlay_data_end = .);
        }


        .d_simple_decode {
            PROVIDE(mode_smpl_dec_ovly_start = .);
            *(.mode_smpl_dec_data);
            PROVIDE(mode_smpl_dec_ovly_end = .);
        }


        .ar_trans_module {
            PROVIDE(ar_trans_data_start = .);
            *(.ar_trans_data);
            PROVIDE(ar_trans_data_end = .);
            PROVIDE(rf_radio_data_start = .);
            *(.rf_radio_data);
            *(.padv_trans_data);
            PROVIDE(rf_radio_data_end = .);
        }

    } > ram0
    __overlay_a_end = .;

    __overlay_b_start = .;
    OVERLAY . :
    AT(0xB0000000) SUBALIGN(4)
    {
        .d_fat_tmp {

            *(.fat_tmp_buf)
        }

        .d_song_speed {
            PROVIDE(song_speed_buf_start = .);
            . = ALIGN(4);
            *(.song_sp_data)
            PROVIDE(song_speed_buf_end = .);
        }
        .d_voicechanger {
            PROVIDE(voicechanger_buf_start = .);
            . = ALIGN(4);
            *(.voicechanger_data)
            PROVIDE(voicechanger_buf_end = .);
        }
        .d_a {
            . = MAX(voicechanger_buf_end, song_speed_buf_end);
            PROVIDE(a_buf_start = .);
            *(.a_data);
            PROVIDE(a_buf_end = .);
        }
        .d_f1a {
            . = a_buf_end;
            PROVIDE(f1a_1_buf_start = .);
            *(.f1a_1_buf);
            PROVIDE(f1a_1_buf_end = .);
            PROVIDE(f1a_2_buf_start = .);
            *(.f1a_2_buf);
            PROVIDE(f1a_2_buf_end = .);

            PROVIDE(f1a_buf_start = .);
            *(.f1a_data);
            PROVIDE(f1a_buf_end = .);
        }
        .d_ump3 {
            . = a_buf_end;
            PROVIDE(ump3_buf_start = .);
            *(.ump3_data);
            PROVIDE(ump3_buf_end = .);
        }
        .d_opus {
            . = a_buf_end;
            PROVIDE(opus_buf_start = .);
            *(.opus_dec_data);
            PROVIDE(opus_buf_end = .);
        }
        .d_ima {
            . = a_buf_end;
            PROVIDE(ima_buf_start = .);
            *(.ima_dec_data);
            PROVIDE(ima_buf_end = .);
        }
        .d_speex {
            . = a_buf_end;
            PROVIDE(speex_buf_start = .);
            *(.speex_dec_data);
            PROVIDE(speex_buf_end = .);
        }
        .d_sbc {
            . = a_buf_end;
            PROVIDE(sbc_buf_start = .);
            *(.sbc_dec_data);
            PROVIDE(sbc_buf_end = .);
        }
        .d_jla_lw {
            . = a_buf_end;
            PROVIDE(jla_lw_buf_start = .);
            *(.jla_lw_dec_data);
            PROVIDE(jla_lw_buf_end = .);
        }

        .d_midi {
            . = a_buf_end;
            PROVIDE(midi_buf_start = .);
            *(.midi_buf);
            PROVIDE(midi_buf_end = .);
            PROVIDE(midi_ctrl_buf_start = .);
            *(.midi_ctrl_buf);
            PROVIDE(midi_ctrl_buf_end = .);
        }

        .d_mp3_st {


            PROVIDE(mp3_st_buf_start = .);
            *(.mp3_st_data);
            PROVIDE(mp3_st_buf_end = .);
        }
        .d_wav {
            . = a_buf_end;
            PROVIDE(wav_buf_start = .);
            *(.wav_data);
            PROVIDE(wav_buf_end = .);
        }

        .d_rec {
            rec_data_start = .;
            *(.rec_data)
            rec_data_end = .;
        }
        .d_enc_a {
            . = rec_data_end;
            *(.enc_a_data)
            enc_a_data_end = .;
        }
        .d_enc_mp3 {
            . = rec_data_end;
            *(.enc_mp3_data)
            enc_mp3_data_end = .;
        }
        .d_enc_ump3 {
            . = rec_data_end;
            *(.enc_ump3_data)
            enc_ump3_data_end = .;
        }
        .d_enc_opus {
            . = rec_data_end;
            *(.enc_opus_data)
            enc_opus_data_end = .;
        }
        .d_enc_speex {
            . = rec_data_end;
            *(.enc_speex_data)
            enc_speex_data_end = .;
        }
        .d_enc_ima {
            . = rec_data_end;
            *(.enc_ima_data)
            enc_ima_data_end = .;
        }
        .d_enc_sbc {
            . = rec_data_end;
            *(.enc_sbc_data)
            enc_sbc_data_end = .;
        }
        .d_enc_jla_lw {
            . = rec_data_end;
            *(.enc_jla_lw_data)
            enc_jla_lw_data_end = .;
        }

        .d_linein {
            PROVIDE(mode_linein_overlay_data_start = .);
            *(.digital_linein_data);
            PROVIDE(mode_linein_overlay_data_end = .);
        }

        .d_usb_slave {
            PROVIDE(mode_pc_overlay_data_start = .);
            *(.uac_var);
            *(.uac_rx);
            *(.uac_spk);
            *(.mass_storage);
            *(.hid_config_var);
            *(.usb_msd_dma);
            *(.usb_hid_dma);
            *(.usb_iso_dma);
            *(.usb_cdc_data);
            *(.usb_descriptor);
            *(.usb_config_var);
            PROVIDE(mode_pc_overlay_data_end = .);
        }
        .d_norflash_cache {
            . = mode_pc_overlay_data_end;
            *(.norflash_cache_buf)
            norflash_cache_buf_end = .;
        }

        .update_tmp_buf {

            . = ALIGN(4);
            *(.uart_update_buf0)
            . = ALIGN(4);
            *(.uart_update_buf1)
            . = ALIGN(4);
            *(.rcsp_buf)
        }

        .d_loud_speaker {
            PROVIDE(mode_loud_spk_overlay_data_start = .);
            . = ALIGN(4);
            *(.speaker_data);
            PROVIDE(mode_loud_spk_overlay_data_end = .);
        }
        .d_howling {
            . = mode_loud_spk_overlay_data_end;
            PROVIDE(howling_data_start = .);
            . = ALIGN(4);
            *(.howling_data);
            PROVIDE(howling_data_end = .);
        }
        .d_realtime_sp {
            . = howling_data_end;
            PROVIDE(realtime_sp_data_start = .);
            . = ALIGN(4);
            *(.sp_data);
            PROVIDE(realtime_sp_data_end = .);
        }
        .d_echo {
            . = howling_data_end;
            PROVIDE(echo_data_start = .);
            . = ALIGN(4);
            *(.echo_data);
            PROVIDE(echo_data_end = .);
        }
        .d_voice_pitch {
            . = howling_data_end;
            PROVIDE(vp_data_start = .);
            . = ALIGN(4);
            *(.vp_data);
            PROVIDE(vp_data_end = .);
        }

        .d_pcm_eq {
            . = howling_data_end;
            PROVIDE(pcm_eq_start = .);
            . = ALIGN(4);
            *(.pcm_eq_data);
            PROVIDE(pcm_eq_end = .);
        }
    } > ram0
    __overlay_b_end = .;
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



        btstack_code_start = .;

        . = ALIGN(4);

        a2dp_source_media_codec_begin = .;
        KEEP(*(.a2dp_source_media_codec))
            a2dp_source_media_codec_end = .;
        a2dp_sink_media_probe_begin = .;
        KEEP(*(.a2dp_sink_media_probe))
            a2dp_sink_media_probe_end = .;

        a2dp_sink_media_codec_begin = .;
        KEEP(*(.a2dp_sink_media_codec))
            a2dp_sink_media_codec_end = .;

        a2dp_event_handler_begin = .;
        KEEP(*(.a2dp_event_handler))
            a2dp_event_handler_end = .;

        sdp_record_item_begin = .;
        KEEP(*(.sdp_record_item))
            sdp_record_item_end = .;

        bt_sleep_begin = .;
        KEEP(*(.bt_sleep))
            bt_sleep_end = .;

        BTSTACK_LE_CODE_START = .;
  *(.bt_stack_const)
  *(.bt_stack_code)
        BTSTACK_LE_CODE_SIZE = ABSOLUTE(. - BTSTACK_LE_CODE_START);

        BTSTACK_LE_DB_CODE_START = .;
        *(.ble_db_const)
        *(.ble_db_code)
        BTSTACK_LE_DB_CODE_SIZE = ABSOLUTE(. - BTSTACK_LE_DB_CODE_START);

        BTSTACK_LE_ATT_CODE_START = .;
        *(.ble_att_const)
        *(.ble_att_code)
        *(.ble_gatt_const)
        *(.ble_gatt_code)
        BTSTACK_LE_ATT_CODE_SIZE = ABSOLUTE(. - BTSTACK_LE_ATT_CODE_START);

        BTSTACK_LE_SM_CODE_START = .;
        *(.ble_sm_const)
        *(.ble_sm_code)
        BTSTACK_LE_SM_CODE_SIZE = ABSOLUTE(. - BTSTACK_LE_SM_CODE_START);


        BTSTACK_LE_HOST_MESH_CODE_START = .;
        *(.ble_mesh_code)
        *(.ble_mesh_tinycrypt_code)

        *(.ble_mesh_const)
        *(.ble_mesh_tinycrypt_const)
        BTSTACK_LE_HOST_MESH_CODE_SIZE = ABSOLUTE(. - BTSTACK_LE_HOST_MESH_CODE_START);

        btstack_code_end = .;
  . = ALIGN(4);


        BTSTACK_CODE_TOTAL_SIZE = btstack_code_end - btstack_code_start;

  . = ALIGN(4);

        btctler_code_start = .;
        BTCTLER_CONTROLLER_CODE_START = .;
  *(.bt_rf_text_const)
  *(.bt_rf_text)
        BTCTLER_CONTROLLER_CODE_SIZE = ABSOLUTE(. - BTCTLER_CONTROLLER_CODE_START);

        BTCTLER_CONTROLLER_HCI_CODE_START = .;
  *(.vendor_manager_text_const)
  *(.vendor_manager_text)
  *(.hci_controller_text_const)
  *(.hci_controller_text)
        BTCTLER_CONTROLLER_HCI_CODE_SIZE = ABSOLUTE(. - BTCTLER_CONTROLLER_HCI_CODE_START);

        BTCTLER_LE_CONTROLLER_BLE_RF_CODE_START = .;
  *(.ble_rf_text_const)
  *(.ble_rf_text)
        BTCTLER_LE_CONTROLLER_BLE_RF_CODE_SIZE = ABSOLUTE(. - BTCTLER_LE_CONTROLLER_BLE_RF_CODE_START);

        BTCTLER_LE_CONTROLLER_BLE_LL_CODE_START = .;
     *(.ble_ll_text_const)
     *(.ble_ll_text)
        BTCTLER_LE_CONTROLLER_BLE_LL_CODE_SIZE = ABSOLUTE(. - BTCTLER_LE_CONTROLLER_BLE_LL_CODE_START);

        BTCTLER_LE_CONTROLLER_BLE_HCI_CODE_START = .;
  *(.ble_hci_text_const)
  *(.ble_hci_text)

        btctler_code_end = .;
        BTCTLER_LE_CONTROLLER_BLE_HCI_CODE_SIZE = ABSOLUTE(. - BTCTLER_LE_CONTROLLER_BLE_HCI_CODE_START);
        BTCTLER_LE_CONTROLLER_CODE_SIZE = BTCTLER_LE_CONTROLLER_BLE_RF_CODE_SIZE + BTCTLER_LE_CONTROLLER_BLE_LL_CODE_SIZE + BTCTLER_LE_CONTROLLER_BLE_HCI_CODE_SIZE;

        BTCTLER_CL_CODE_START = .;
        BTCTLER_CL_CODE_SIZE = ABSOLUTE(. - BTCTLER_CL_CODE_START);
        BTCTLER_CODE_TOTAL = (btctler_code_end - btctler_code_start);


        . = ALIGN(32);


 } >app_code


}




SECTIONS
{
    .data : ALIGN(4)
    {
        update_data_start = .;

        update_data_end = .;
    } > ram0

    .bss (NOLOAD) :ALIGN(4)
    {
        update_bss_start = .;

        *(.update_bss)
        update_bss_end = .;
    } > ram0


    .app_code : ALIGN(4)
    {
  update_code_start = .;

  *(.bt_updata_ram_code)
  *(.update_const)
  *(.update_code)

  update_code_end = .;

  . = ALIGN(4);

 } >app_code


    UPDATE_CODE_TOTAL_SIZE = update_code_end - update_code_start;
}



bss_begin = ADDR(.bss);
bss_size = SIZEOF(.bss);


bss_size1 = _system_data_end - _system_data_begin;
bss_begin1 = _system_data_begin;

text_begin = ADDR(.app_code);
text_size = SIZEOF(.app_code);
text_end = text_begin + text_size;

data_addr = ADDR(.data) ;
data_begin = text_end ;
data_size = SIZEOF(.data) + SIZEOF(.debug_data);
bt_app_size = bt_app_end - bt_app_start;
heap_size = _free_end - _free_start;
