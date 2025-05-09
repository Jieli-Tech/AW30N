#include "app_config.h"
#include "printf.h"
#if USB_DEVICE_CLASS_CONFIG & AUDIO_CLASS
#include "usb/usb_config.h"
#include "usb/device/usb_stack.h"
#include "usb/device/uac_audio.h"
#include "usb/usr/usb_audio_interface.h"
#include "usb/usr/usb_mic_interface.h"
#include "uac_stream.h"
#include "usb_config.h"
#include "circular_buf.h"
#include "sound_mge.h"
#include "audio_dac_api.h"

#define LOG_TAG_CONST       USB
#define LOG_TAG             "[UAC]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE
#include "log.h"
#include "uart.h"
#include "msg.h"
/* #include "dac_api.h" */

#define     UAC_DEBUG_ECHO_MODE 0


static volatile u8 speaker_stream_is_open = 0;
struct uac_speaker_handle {
    cbuffer_t cbuf;
    void *buffer;
    u32 sr;
    u16 in_cnt;
    u8 not_first;
    /* u8 ch; */
};
#define UAC_BUFFER_SIZE     (96*2*20)
#define UAC_BUFFER_MAX		(UAC_BUFFER_SIZE * 50 / 100)

/* static struct uac_speaker_handle *uac_speaker = NULL; */

static struct uac_speaker_handle uac_speaker SEC(.uac_var);
static u8 uac_rx_buffer[UAC_BUFFER_SIZE] ALIGNED(4) SEC(.uac_rx);

sound_out_obj uac_spk_sound AT(.uac_var);

u32 uac_speaker_stream_length()
{
    return UAC_BUFFER_SIZE;
}
u32 uac_speaker_stream_size()
{
    if (!speaker_stream_is_open) {
        return 0;
    }

    return cbuf_get_data_size(&uac_speaker.cbuf);

}

void uac_speaker_stream_buf_clear(void)
{
    if (speaker_stream_is_open) {
        cbuf_clear(&uac_speaker.cbuf);
    }
}

void set_uac_speaker_rx_handler(void *priv, void (*rx_handler)(int, void *, int))
{
}

void uac_speaker_stream_write(const u8 *obuf, u32 len)
{
    /* printf_buf(obuf,16); */
#if (SPK_AUDIO_RATE > 0xFFFF)   //88.2k, 96K
    if (len) {
        len = SPK_FRAME_LEN;
    }
#endif
    if (speaker_stream_is_open) {
        uac_speaker.in_cnt += len / 2;
        usb_slave_sound_write(obuf, len);
    } else {
        log_char('n');
    }
}

#if 1
int uac_speaker_read(void *priv, void *data, u32 len)
{
    int r_len;
    int err = 0;
    return len;
    local_irq_disable();
    if (!speaker_stream_is_open) {
        local_irq_enable();
        return 0;
    }

    r_len = cbuf_get_data_size(&uac_speaker.cbuf);
    if (r_len) {
        r_len = r_len > len ? len : r_len;
        r_len = cbuf_read(&uac_speaker.cbuf, data, r_len);
        if (!r_len) {
            putchar('U');
        }
    }

    local_irq_enable();
    return r_len;
}
#endif
void uac_speaker_stream_open(u32 samplerate, u32 ch)
{
#if (SPK_AUDIO_RATE >= 0xFFFF)   //88.2k, 96K
    samplerate = SPK_AUDIO_RATE;
#endif
#if (USB_DEVICE_CLASS_CONFIG & SPEAKER_CLASS) == SPEAKER_CLASS
    log_info(" uac_speaker_stream_open!\n");
    if (speaker_stream_is_open) {
        return;
    }
    log_info("%s", __func__);


    uac_speaker.buffer = uac_rx_buffer;


    cbuf_init(&uac_speaker.cbuf, uac_speaker.buffer, UAC_BUFFER_SIZE);

    sound_out_init(&uac_spk_sound, (void *)&uac_speaker.cbuf, (SPK_CHANNEL == 2) ? B_STEREO : 0);
    /* sound_out_init(&uac_spk_sound, (void *)&uac_speaker.cbuf, B_STEREO); */
    /* memset(&uac_speaker, 0, sizeof(struct uac_speaker_handle)); */
    /* uac_spk_sound.p_obuf = &uac_speaker.cbuf; */
    /* uac_spk_sound.info |= B_STEREO; */

    uac_speaker.sr =  samplerate;
    /* uac_speaker.ch = ch; */
    dac_sr_api(samplerate);

    usb_slave_sound_open(&uac_spk_sound, uac_speaker.sr);
    post_event(EVENT_PC_SPK);
    speaker_stream_is_open = 1;
#endif
}

void uac_speaker_stream_close()
{
    if (speaker_stream_is_open == 0) {
        return;
    }

    log_info("%s", __func__);
    speaker_stream_is_open = 0;
    usb_slave_sound_close(&uac_spk_sound);
    /* struct sys_event event;                    */
    /* event.type = SYS_DEVICE_EVENT;             */
    /* event.arg = (void *)DEVICE_EVENT_FROM_UAC; */
    /* event.u.dev.event = USB_AUDIO_PLAY_CLOSE;  */
    /* event.u.dev.value = (int)0;                */

    /* sys_event_notify(&event);                  */
}

u32 uac_speaker_stream_1s(void)
{
    if (0 != speaker_stream_is_open) {
        if (0 != uac_speaker.in_cnt) {
            if (0 == uac_speaker.not_first) {
                uac_speaker.in_cnt = 0;
                uac_speaker.not_first = 1;
                log_info("uac_1s first\n");
            } else {
                u32 res = uac_speaker.in_cnt;
                uac_speaker.in_cnt = 0;
                return res;
            }
        }
    }
    return 0;
}

int __attribute__((weak)) uac_vol_switch(int vol)
{

    u16 valsum = vol * 32 / 100;

    if (valsum > 31) {
        valsum = 31;
    }
    return valsum;
}

int uac_get_spk_vol()
{
    return 100;
}
void uac_spk_vol(u8 vol_l, u8 vol_r)
{
    log_info("vol_l: %d , vol_r: %d", vol_l, vol_r);
    stereo_dac_vol(0, vol_l, vol_r);
}
static u32 mic_stream_is_open;
static u32 last_spk_l_vol       AT(.uac_var);
static u32 last_spk_r_vol       AT(.uac_var);
static u32 last_mic_vol         AT(.uac_var);
void uac_mute_volume(u32 type, u32 l_vol, u32 r_vol)
{
    /* struct sys_event event;                    */
    /* event.type = SYS_DEVICE_EVENT;             */
    /* event.arg = (void *)DEVICE_EVENT_FROM_UAC; */

    int l_valsum, r_valsum;
    /* static u32 last_spk_l_vol = (u32) - 1, last_spk_r_vol = (u32) - 1; */
    /* static u32 last_mic_vol = (u32) - 1; */

    l_valsum = uac_vol_switch(l_vol);
    r_valsum = uac_vol_switch(r_vol);
    switch (type) {
#if (USB_DEVICE_CLASS_CONFIG & MIC_CLASS)
    case MIC_FEATURE_UNIT_ID: //MIC
        /* if (mic_stream_is_open == 0) { */
        /* return ; */
        /* } */
        if (l_vol == last_mic_vol) {
            return;
        }
        last_mic_vol = l_vol;
        uac_mic_vol(l_valsum, r_valsum);
        break;
#endif
    case SPK_FEATURE_UNIT_ID: //SPK
        /* if (speaker_stream_is_open == 0) { */
        /* return; */
        /* } */
        if (l_vol == last_spk_l_vol && r_vol == last_spk_r_vol) {
            return;
        }
        last_spk_l_vol = l_vol;
        last_spk_r_vol = r_vol;


#if (SPK_CHANNEL == 1)
        uac_spk_vol(l_valsum, l_valsum);
#elif  (SPK_CHANNEL == 2)
        uac_spk_vol(l_valsum, r_valsum);
#endif
        break;
    default:
        break;
    }

    /* log_info("type :%d; lvol:%d; rvol:%d;", type, l_valsum, r_valsum); */
    /* event.u.dev.value = (int)(r_valsum << 16 | l_valsum); */
    /* sys_event_notify(&event);                             */
}


static const s16 sin_48k[] = {
    0, 2139, 4240, 6270, 8192, 9974, 11585, 12998,
    14189, 15137, 15826, 16244, 16384, 16244, 15826, 15137,
    14189, 12998, 11585, 9974, 8192, 6270, 4240, 2139,
    0, -2139, -4240, -6270, -8192, -9974, -11585, -12998,
    -14189, -15137, -15826, -16244, -16384, -16244, -15826, -15137,
    -14189, -12998, -11585, -9974, -8192, -6270, -4240, -2139
};

static int (*mic_tx_handler)(int, void *, int) SEC(.uac_rx);
static int uac_mic_fix_data(u8 *buf, u32 len)
{

#if MIC_CHANNEL == 2
    u16 *l_ch = (u16 *)buf;
    u16 *r_ch = (u16 *)buf;
    r_ch++;
    for (int i = 0; i < len / 2; i++) {
        *l_ch = sin_48k[i];
        *r_ch = sin_48k[i];
        l_ch += 2;
        r_ch += 2;
    }
#else
    memcpy(buf, sin_48k, len);
#endif

    return len;
}
static int uac_mic_echo_data(u8 *buf, u32 len)
{
#if MIC_CHANNEL == 2
    uac_speaker_read(NULL, buf, len);
#if MIC_AUDIO_RES==16
    u16 *r_ch = (u16 *)buf;
    r_ch++;
    for (int i = 0; i < len / 4; i++) {
        *r_ch = sin_48k[i];
        r_ch += 2;
    }
#else
    //16bit ---> 24bit
    u8 *p = (u8 *)sin_48k;
    u32 j = 0;
    for (int i = 0 ; i < len ; i += 6) {
        buf[i] = 0;
        buf[i + 1] = p[j];
        buf[i + 2] = p[j + 1];
        j += 2;
    }
#endif
#endif
    return len;
}
int uac_mic_stream_read(u8 *buf, u32 len)
{
#if (MIC_AUDIO_RATE > 0xFFFF)   //88.2k, 96K
    if (len) {
        len = MIC_FRAME_LEN;
    }
#endif
#if USB_DEVICE_CLASS_CONFIG & MIC_CLASS
    if (mic_stream_is_open == 0) {
        memset(buf, 0, len);
        return len;
    }
    return usb_slave_mic_read(buf, len);
#else
    return 0;
#endif

#if 0//48K 1ksin

    return uac_mic_fix_data(buf, len);

#elif   UAC_DEBUG_ECHO_MODE

    return uac_mic_echo_data(buf, len);
#else

    if (mic_tx_handler) {
        return mic_tx_handler(0, buf, len);
    } else {
        putchar('N');
        return 0;
    }
#endif

}

#if 0
//16bit ---> 24bit
int rlen = mic_tx_handler(0, tmp_buf, len / 3 * 2);
rlen /= 2; //sampe point
for (int i = 0 ; i < rlen ; i++)
{
    buf[i * 3] = 0;
    buf[i * 3 + 1] = tmp_buf[i * 2];
    buf[i * 3 + 2] = tmp_buf[i * 2 + 1];
}
#endif

void set_uac_mic_tx_handler(void *priv, int (*tx_handler)(int, void *, int))
{
    mic_tx_handler = tx_handler;
}

u32 uac_mic_stream_open(u32 samplerate, u32 frame_len, u32 ch)
{
#if (MIC_AUDIO_RATE >= 0xFFFF)   //88.2k, 96K
    samplerate = MIC_AUDIO_RATE;
#endif
#if USB_DEVICE_CLASS_CONFIG & MIC_CLASS
    if (mic_stream_is_open) {
        return 0;
    }

    mic_stream_is_open = 1;
    return usb_slave_mic_open(samplerate, frame_len, ch);
    mic_tx_handler = NULL;
    log_info("%s", __func__);

#endif
    mic_stream_is_open = 1;
    return 0;
}
void uac_mic_stream_close()
{
#if USB_DEVICE_CLASS_CONFIG & MIC_CLASS
    if (mic_stream_is_open == 0) {
        return ;
    }
    log_info("%s", __func__);
    mic_stream_is_open = 0;
    usb_slave_mic_close();
    return;
    /* usb_mic_uninit(); */
#endif
}

#if 0
struct uac_info_t _uac_info = {
    .uac_speaker_stream_open = uac_speaker_stream_open,
    .uac_speaker_stream_write = uac_speaker_stream_write,
    .uac_speaker_stream_close = uac_speaker_stream_close,
    .uac_mute_volume = uac_mute_volume,
    .uac_mic_stream_open = uac_mic_stream_open,
    .uac_mic_stream_close = uac_mic_stream_close,
    .uac_get_spk_vol = uac_get_spk_vol,
    .uac_mic_stream_read = uac_mic_stream_read,
    .spk_audio_rate = (u16)SPK_AUDIO_RATE,
    .spk_channle = SPK_CHANNEL,
    .spk_audio_res = SPK_AUDIO_RES,

    .mic_channle = MIC_CHANNEL,
    .mic_audio_rate = (u16)MIC_AUDIO_RATE,
    .mic_audio_res = MIC_AUDIO_RES,
};
#endif
struct uac_info_t _uac_info AT(.uac_var);

void uac_init(void)
{
    last_spk_l_vol = (u32) - 1;
    last_spk_r_vol = (u32) - 1;
    last_mic_vol = (u32) - 1;

    memset((void *)&_uac_info, 0, sizeof(struct uac_info_t));
    _uac_info.uac_speaker_stream_open = uac_speaker_stream_open;
    _uac_info.uac_speaker_stream_write = uac_speaker_stream_write;
    _uac_info.uac_speaker_stream_close = uac_speaker_stream_close;
    _uac_info.uac_mute_volume = uac_mute_volume;
    _uac_info.uac_mic_stream_open = uac_mic_stream_open;
    _uac_info.uac_mic_stream_close = uac_mic_stream_close;
    _uac_info.uac_get_spk_vol = uac_get_spk_vol;
    _uac_info.uac_mic_stream_read = uac_mic_stream_read;
    _uac_info.spk_audio_rate = (u32)SPK_AUDIO_RATE;
    _uac_info.spk_channle = SPK_CHANNEL;
    _uac_info.spk_audio_res = SPK_AUDIO_RES;

    _uac_info.mic_channle = MIC_CHANNEL;
    _uac_info.mic_audio_rate = (u32)MIC_AUDIO_RATE;
    _uac_info.mic_audio_res = MIC_AUDIO_RES;

    _uac_info.speaker_dma_buffer = usb_get_ep_buffer(0, SPK_ISO_EP_OUT);
    _uac_info.mic_dma_buffer = usb_get_ep_buffer(0, MIC_ISO_EP_IN | USB_DIR_IN);

    set_usb_mic_info(NULL);//清除USB_MIC管理句柄
}

void uac_release_api(void)
{
#if (USB_DEVICE_CLASS_CONFIG & SPEAKER_CLASS) == SPEAKER_CLASS
    uac_speaker_stream_close();
#endif
#if (USB_DEVICE_CLASS_CONFIG & MIC_CLASS) == MIC_CLASS
    uac_mic_stream_close();
#endif
    set_usb_mic_func(NULL, NULL);//注册auadc mic
}

bool get_usb_mic_status()
{
    return mic_stream_is_open;
}

#endif

