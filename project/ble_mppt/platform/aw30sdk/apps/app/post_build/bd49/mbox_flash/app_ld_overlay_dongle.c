// *INDENT-OFF*
    __overlay_dongle_start = .;
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
        .ar_trans_module {
            . = norflash_cache_buf_end;
            PROVIDE(ar_trans_data_start = .);
            *(.ar_trans_data);
            PROVIDE(ar_trans_data_end = .);
        }
        .d_opus {
            . = ar_trans_data_end;
            PROVIDE(opus_buf_start = .);
            *(.opus_dec_data);
            PROVIDE(opus_buf_end = .);
        }
        .d_ima {
            . = ar_trans_data_end;
            PROVIDE(ima_buf_start = .);
            *(.ima_dec_data);
            PROVIDE(ima_buf_end = .);
        }
        .d_sbc {
            . = ar_trans_data_end;
            PROVIDE(sbc_buf_start = .);
            *(.sbc_dec_data)
            PROVIDE(sbc_buf_end = .);
        }
        .d_jla_lw {
            . = ar_trans_data_end;
            PROVIDE(jla_lw_buf_start = .);
            *(.jla_lw_dec_data);
            PROVIDE(jla_lw_buf_end = .);
        }
        .d_speex {
            . = ar_trans_data_end;
            PROVIDE(speex_buf_start = .);
            *(.speex_dec_data);
            PROVIDE(speex_buf_end = .);
        }
        .update_tmp_buf {
            . = norflash_cache_buf_end;
            . = ALIGN(4);
            *(.uart_update_buf0)
            . = ALIGN(4);
            *(.uart_update_buf1)
            . = ALIGN(4);
            *(.rcsp_buf)
        }
    } > ram0
    __overlay_dongle_end = .;
