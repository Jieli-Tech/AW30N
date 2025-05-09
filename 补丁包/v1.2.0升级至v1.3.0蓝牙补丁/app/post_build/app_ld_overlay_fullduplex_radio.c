// *INDENT-OFF*
    __overlay_app_handle_start = .;
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

        //app
        /* 小音箱应用 */
        .d_music_play {
            PROVIDE(mode_music_overlay_data_start = .);
            *(.mode_music_overlay_data);
            /* *(.usb_h_dma); */
            *(.fat_buf);
            PROVIDE(mode_music_overlay_data_end = .);
        }
        .d_fat_tmp {
            . = mode_music_overlay_data_end;
            *(.fat_tmp_buf)
        }

        /* 简单解码应用 */
        .d_simple_decode {
            PROVIDE(mode_smpl_dec_ovly_start = .);
            *(.mode_smpl_dec_data);
            PROVIDE(mode_smpl_dec_ovly_end = .);
        }
        /* 无线音频传输应用（包含对讲机和遥控器应用） */
        .ar_trans_module {
            PROVIDE(ar_trans_data_start = .);
            *(.ar_trans_data);
            PROVIDE(ar_trans_data_end = .);
            PROVIDE(rf_radio_data_start = .);
            *(.rf_radio_data);
            PROVIDE(rf_radio_data_end = .);
        }
        .d_rec {
            . = rf_radio_data_end;
            rec_data_start = .;
            *(.rec_data)
            rec_data_end = .;
        }
    } > ram0
    __overlay_app_handle_end = .;

    __overlay_enc_data_start = .;
    OVERLAY . :
    AT(0xB0000000) SUBALIGN(4)
    {
        .d_enc_a {
            *(.enc_a_data)
            enc_a_data_end = .;
        }
        .d_enc_mp3 {
            *(.enc_mp3_data)
            enc_mp3_data_end = .;
        }
        .d_enc_ump3 {
            *(.enc_ump3_data)
            enc_ump3_data_end = .;
        }
        .d_enc_opus {
            *(.enc_opus_data)
            enc_opus_data_end = .;
        }
        .d_enc_speex {
            *(.enc_speex_data)
            enc_speex_data_end = .;
		}
        .d_enc_ima {
            *(.enc_ima_data)
            enc_ima_data_end = .;
        }
        .d_enc_sbc {
            *(.enc_sbc_data)
            enc_sbc_data_end = .;
        }
    } > ram0
    __overlay_enc_data_end = .;

    __overlay_dec_data_start = .;
	OVERLAY :
    AT(0xC0000000) SUBALIGN(4)
    {
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
            /* . = norflash_cache_buf_end; */
            . = ALIGN(4);
            *(.uart_update_buf0);
            . = ALIGN(4);
            *(.uart_update_buf1);
            . = ALIGN(4);
            *(.rcsp_buf);
        }
        .d_linein {
            PROVIDE(mode_linein_overlay_data_start = .);
            *(.digital_linein_data);
            PROVIDE(mode_linein_overlay_data_end = .);
        }
        .d_loud_speaker {
            PROVIDE(mode_loud_spk_overlay_data_start = .);
            . = ALIGN(4);
            *(.speaker_data);
            PROVIDE(mode_loud_spk_overlay_data_end = .);
        }
        .d_song_speed {
            . = mode_loud_spk_overlay_data_end;
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
        .d_howling {
            PROVIDE(howling_data_start = .);
            . = ALIGN(4);
            *(.howling_data);
            PROVIDE(howling_data_end = .);
        }
        .d_realtime_sp {
            /* . = howling_data_end; */
            PROVIDE(realtime_sp_data_start = .);
            . = ALIGN(4);
            *(.sp_data);
            PROVIDE(realtime_sp_data_end = .);
        }
        .d_echo {
            /* . = howling_data_end; */
            PROVIDE(echo_data_start = .);
            . = ALIGN(4);
            *(.echo_data);
            PROVIDE(echo_data_end = .);
        }
        .d_voice_pitch {
            /* . = howling_data_end; */
            PROVIDE(vp_data_start = .);
            . = ALIGN(4);
            *(.vp_data);
            PROVIDE(vp_data_end = .);
        }

        .d_pcm_eq {
            /* . = howling_data_end; */
            PROVIDE(pcm_eq_start = .);
            . = ALIGN(4);
            *(.pcm_eq_data);
            PROVIDE(pcm_eq_end = .);
        }
        .d_a {
            PROVIDE(a_buf_start = .);
            *(.a_data);
            PROVIDE(a_buf_end = .);
        }
        .d_f1a {
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
            PROVIDE(ump3_buf_start = .);
            *(.ump3_data);
            PROVIDE(ump3_buf_end = .);
        }
        .d_opus {
            PROVIDE(opus_buf_start = .);
            *(.opus_dec_data);
            PROVIDE(opus_buf_end = .);
        }
        .d_ima {
            PROVIDE(ima_buf_start = .);
            *(.ima_dec_data);
            PROVIDE(ima_buf_end = .);
        }
        .d_speex {
            PROVIDE(speex_buf_start = .);
            *(.speex_dec_data);
            PROVIDE(speex_buf_end = .);
        }
        .d_sbc {
            PROVIDE(sbc_buf_start = .);
            *(.sbc_dec_data);
            PROVIDE(sbc_buf_end = .);
        }
        .d_midi {
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
            PROVIDE(wav_buf_start = .);
            *(.wav_data);
            PROVIDE(wav_buf_end = .);
        }
    } > ram0
    __overlay_dec_data_end = .;
