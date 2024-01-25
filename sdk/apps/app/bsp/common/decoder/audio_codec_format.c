#include "audio_codec_format.h"
#include "decoder_msg_tab.h"
#include "app_modules.h"

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[codec]"
#include "log.h"

const u8 audio_codec_list[][2] = {
#if DECODER_UMP3_EN
    {FORMAT_UMP3,   INDEX_UMP3},
#endif
#if DECODER_OPUS_EN
    {FORMAT_OPUS,   INDEX_OPUS},
#endif
#if DECODER_IMA_EN
    {FORMAT_IMA,    INDEX_IMA},
#endif
#if DECODER_SBC_EN
    {FORMAT_SBC,    INDEX_SBC},
#endif
#if DECODER_SPEEX_EN
    {FORMAT_SPEEX,  INDEX_SPEEX},
#endif
};

u32 select_codec(AUDIO_FORMAT enc_type)
{
    u32 res = -1;
    for (int i = 0; i < ARRAY_SIZE(audio_codec_list); i++) {
        if (enc_type == audio_codec_list[i][0]) {
            res = audio_codec_list[i][1];
            break;
        }
    }
    return res;
}
