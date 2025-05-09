#pragma bss_seg(".record_mode.data.bss")
#pragma data_seg(".record_mode.data")
#pragma const_seg(".record_mode.text.const")
#pragma code_seg(".record_mode.text")
#pragma str_literal_override(".record_mode.text.const")

#include "record_mode.h"
#include "device.h"
#include "device_app.h"
#include "vfs.h"
#include "msg.h"
#include "ui_api.h"
#include "hot_msg.h"
#include "encoder_mge.h"
#include "encoder_file.h"
#include "music_play.h"
#include "play_file.h"
#include "app.h"
#include "jiffies.h"
#include "sys_memory.h"

#include "decoder_api.h"
#include "decoder_msg_tab.h"

#include "audio.h"
#include "audio_dac_api.h"
#include "audio_adc_api.h"
#include "audio_adc.h"

#if ENCODER_UMP3_EN
#include "ump3_encoder.h"
#define ENCODER_API  ump3_encode_api
#define DEC_TYPE	 (BIT_UMP3 | BIT_SPEED)
#elif ENCODER_MP3_EN
#include "mp3_encoder.h"
#define ENCODER_API  mp3_encode_api
#define DEC_TYPE	 BIT_MP3_ST
#elif ENCODER_A_EN
#include "a_encoder.h"
#define ENCODER_API  a_encode_api
#define DEC_TYPE	 (BIT_A | BIT_SPEED)
#endif

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[rec]"
#include "log.h"

#if RECORD_MODE_EN

#define RECORD_AUDIO_ADC_SR     RECORD_ADC_SR_32K
Encode_Control record_obj;

void record_app(void)
{
    sysmem_write_api(SYSMEM_INDEX_SYSMODE, &work_mode, sizeof(work_mode));
    int msg[2];
    u32 err = 0;
    dec_obj *p_dec_obj = 0;
    u16 decode_type = DEC_TYPE;
    key_table_sel(record_key_msg_filter);
    decoder_init();

    memset(&record_obj, 0, sizeof(record_obj));

    while (1) {
        err = get_msg(2, &msg[0]);
        bsp_loop();
        if (MSG_NO_ERROR != err) {
            msg[0] = NO_MSG;
            log_info("get msg err 0x%x\n", err);
        }

        switch (msg[0]) {
        case MSG_RECODE_START:
            if (ENC_ING == record_obj.enc_status) {
                /* 结束录音并播放录音 */
                encode_stop(&record_obj);
                post_msg(1, MSG_PP);
            } else {
                /* 开始录音 */
                decoder_stop(p_dec_obj, NEED_WAIT, 0);
                encode_file_fs_close(&record_obj);
                encode_start(&record_obj);
                log_info("dev:%d fs_name:%s\n", record_obj.dev_index, record_obj.fs_name);
            }
            break;
        case MSG_PP:
            encode_stop(&record_obj);
            decoder_stop(p_dec_obj, NEED_WAIT, 0);
            dec_obj *(*enc_file_decode)(Encode_Control * obj, u16 dec_type);

            if (0 == (strcmp(record_obj.fs_name, "fat"))) {
                enc_file_decode = fatfs_enc_file_decode;
            } else if (0 == (strcmp(record_obj.fs_name, "norfs"))) {
                enc_file_decode = norfs_enc_file_decode;
            } else {
                log_info("record hasn't been started!\n");
                break;
            }
            p_dec_obj = enc_file_decode(&record_obj, decode_type);
            if (NULL == p_dec_obj) {
                log_info("record file decode fail!\n");
                encode_file_fs_close(&record_obj);
            }
            break;

        case MSG_REC_SPEED_EN:
            if (decode_type & BIT_MP3_ST) {
                /* 标准MP3解码与变速变调资源互斥 */
                break;
            }
            if (decode_type & BIT_SPEED) {
                log_info("record normal mode \n");
                decode_type &= ~BIT_SPEED;
            } else {
                log_info("record speed mode \n");
                decode_type |= BIT_SPEED;
            }
            break;
        case MSG_WFILE_FULL:
            log_info("MSG_WFILE_FULL\n");
            encode_stop(&record_obj);
            break;
        case MSG_WAV_FILE_END:
        case MSG_WAV_FILE_ERR:
        case MSG_MP3_FILE_END:
        case MSG_MP3_FILE_ERR:
        case MSG_A_FILE_END:
        case MSG_A_FILE_ERR:
            decoder_stop(p_dec_obj, NEED_WAIT, 0);
            encode_file_fs_close(&record_obj);
            break;

        case MSG_CHANGE_WORK_MODE:
            goto __record_app_exit;
        case MSG_500MS:
            UI_menu(MENU_MAIN, 0);
            if (record_obj.enc_status == ENC_ING) {
                log_char('R');
            }
            if ((MUSIC_PLAY != get_decoder_status(p_dec_obj)) && \
                (record_obj.enc_status == ENC_NULL)) {
                sysmem_pre_erase_api();
                app_powerdown_deal(0);
            } else {
                app_powerdown_deal(1);
            }
        default:
            ap_handle_hotkey(msg[0]);
            break;
        }
    }

__record_app_exit:
    key_table_sel(NULL);
    if (ENC_ING == record_obj.enc_status) {
        encode_stop(&record_obj);
    } else {
        decoder_stop(p_dec_obj, NEED_WAIT, 0);
        encode_file_fs_close(&record_obj);
    }
    return;
}

void encode_file_fs_close(Encode_Control *obj)
{
    fs_file_close(&obj->pfile);
    fs_fs_close(&obj->pfs);
    device_close(obj->dev_index);
}

static void encode_stop(Encode_Control *obj)
{
    if (ENC_ING == obj->enc_status) {
        obj->enc_obj = stop_encode_file(obj->enc_obj, 0);
        encode_file_fs_close(obj);
        obj->enc_status = ENC_NULL;
    }
    audio_adc_off_api();
}

const u8 recoder_device_tab[3] = {
    UDISK_INDEX,
    SD0_INDEX,
    INNER_FLASH_RW
};
static int encode_start(Encode_Control *obj)
{
    u32 sr = RECORD_AUDIO_ADC_SR;
    /* u32 sr = dac_sr_read(); */
    /* log_info("adc sr:%d\n", sr); */
    int err = audio_adc_init_api(sr, AUDIO_ADC_MIC, 3000);
    if (0 != err) {
        log_info(" audio adc init fail : 0x%x\n");
        return err;
    }

    u32 online_dev = device_online();
    u8 index = -1;
    for (u8 i = 0; i <= sizeof(recoder_device_tab); i++) {
        index = recoder_device_tab[i];
        if (online_dev & BIT(index)) {
            break;
        }
    }

    const char *p_fs_name = NULL;
    int (*enc_file_create)(Encode_Control * obj);
    if (index > SD0_INDEX) {
        p_fs_name = "norfs";//内置flash录音
        enc_file_create = norfs_enc_file_create;
    } else {
        p_fs_name = "fat";//SD卡、U盘录音
        enc_file_create = fatfs_enc_file_create;
    }
#ifdef ENCODER_API
    obj->dev_index = index;
    strcpy(obj->fs_name, p_fs_name);
    err = enc_file_create(obj);
    if (0 != err) {
        log_info("%s create 0x%x!\n", p_fs_name, err);
        return err;
    }
    record_obj.enc_obj = encoder_file(ENCODER_API, obj->pfile);
#else
    log_info("no format encoder!\n");
    return E_ENC_FORMAT;
#endif

    obj->enc_status = ENC_ING;
    return 0;
}


#endif
