// *INDENT-OFF*
    __overlay_rc_start = .;
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

        .ar_trans_module {
            PROVIDE(ar_trans_data_start = .);
            *(.ar_trans_data);
            PROVIDE(ar_trans_data_end = .);
        }
        .d_rec {
            . = ar_trans_data_end;
            rec_data_start = .;
            *(.rec_data)
            rec_data_end = .;
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

        .update_tmp_buf {
            /* . = norflash_cache_buf_end; */
            . = ALIGN(4);
            *(.uart_update_buf0)
            . = ALIGN(4);
            *(.uart_update_buf1)
            . = ALIGN(4);
            *(.rcsp_buf)
        }
    } > ram0
    __overlay_rc_end = .;
