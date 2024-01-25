// *INDENT-OFF*
ifndef __APP_MODULES_H__
define __APP_MODULES_H__

** 此文件在在服务器编译库时会自动生成，源文件位于apps/app/bsp/common/app_modules_h.c

** 作者: 刘杰
** 日期:2022年11月22日
** 设计目的: 用于在应用代码中控制各种算法模块的使用
** 注意事项：不要在库文件中包含

-
-
** APP方案选择
define RUN_APP_CUSTOM   1
define RUN_APP_RC       0
define RUN_APP_DONGLE   0

-
-
if RUN_APP_CUSTOM
-
-
** A格式解码
#ifdef HAS_A_DECODER
define DECODER_A_EN  1
#else
define DECODER_A_EN  0
#endif

-
** ADPCM-IMA格式解码
#ifdef HAS_IMA_DECODER
define DECODER_IMA_EN  1
#else
define DECODER_IMA_EN  0
#endif

-
** 标准MP3格式解码
#ifdef HAS_MP3_ST_DECODER
define DECODER_MP3_ST_EN 1
#else
define DECODER_MP3_ST_EN 0
#endif


-
** WAV格式解码
#ifdef HAS_WAV_DECODER
define DECODER_WAV_EN 1
#else
define DECODER_WAV_EN 0
#endif


-
** F1A格式解码
#ifdef HAS_F1A_DECODER
define DECODER_F1A_EN  1
define MAX_F1A_CHANNEL HAS_MAX_F1A_NUMBER
#else
define DECODER_F1A_EN  0
define MAX_F1A_CHANNEL 0
#endif

-
** UMP3格式解码
#ifdef HAS_UMP3_DECODER
define DECODER_UMP3_EN 1
#else
define DECODER_UMP3_EN 0
#endif

-
** MIDI格式解码
#ifdef HAS_MIDI_DECODER
define DECODER_MIDI_EN 1
#else
define DECODER_MIDI_EN 0
#endif

-
** MIDI琴格式解码
#ifdef HAS_MIDI_KEYBOARD_DECODER
define DECODER_MIDI_KEYBOARD_EN 1
#else
define DECODER_MIDI_KEYBOARD_EN 0
#endif

-
** OPUS格式解码
#ifdef HAS_OPUS_DECODER
define DECODER_OPUS_EN  0
#else
define DECODER_OPUS_EN  0
#endif

-
** SPEEX格式解码
#ifdef HAS_SPEEX_DECODER
define DECODER_SPEEX_EN  1
#else
define DECODER_SPEEX_EN  0
#endif

-
** SBC格式解码
#ifdef HAS_SBC_DECODER
define DECODER_SBC_EN  1
#else
define DECODER_SBC_EN  0
#endif

-
** mp3格式压缩
#ifdef HAS_MP3_ENCODER
define ENCODER_MP3_EN     1
#if (2 == MP3_ENCODER_SUPPORT_CH)
define ENCODER_MP3_STEREO     1 --  库支持压缩立体声的音源，改为0后只能压缩单声道，但是会节省2K+的RAM消耗
#else
define ENCODER_MP3_STEREO     0
#endif
#else
define ENCODER_MP3_EN     0
#endif


-
** ump3格式压缩
#ifdef HAS_UMP3_ENCODER
define ENCODER_UMP3_EN  1
#else
define ENCODER_UMP3_EN  0
#endif

-
** a格式压缩
#ifdef HAS_A_ENCODER
define ENCODER_A_EN  1
#else
define ENCODER_A_EN  0
#endif

-
** opus格式压缩
#ifdef HAS_OPUS_ENCODER
define ENCODER_OPUS_EN  0
#else
define ENCODER_OPUS_EN  0
#endif

-
** SPEEX格式压缩
#ifdef HAS_SPEEX_ENCODER
define ENCODER_SPEEX_EN  1
#else
define ENCODER_SPEEX_EN  0
#endif

-
** adpcm-ima格式压缩
#ifdef HAS_IMA_ENCODER
define ENCODER_IMA_EN  1
#else
define ENCODER_IMA_EN  0
#endif

-
** SBC格式压缩
#ifdef HAS_SBC_ENCODER
define ENCODER_SBC_EN  1
#else
define ENCODER_SBC_EN  0
#endif

-
** MIO功能使能
#ifdef HAS_MIO_PLAYER
define HAS_MIO_EN  0
#else
define HAS_MIO_EN  0
#endif

-
** 实时SPEED功能(用于扩音)使能
#ifdef HAS_SPEED_EFFECT
define HAS_SPEED_EN  1
#else
define HAS_SPEED_EN  0
#endif

-
** 歌曲SPEED功能(用于解码)使能
#ifdef HAS_SONG_SPEED_EFFECT
define HAS_SONG_SPEED_EN  1
#else
define HAS_SONG_SPEED_EN  0
#endif

-
** 陷波/移频啸叫抑制使能(二选一)
#ifdef HAS_HOWLING_EFFECT
define NOTCH_HOWLING_EN  1
define PITCHSHIFT_HOWLING_EN 2
define HOWLING_SEL   PITCHSHIFT_HOWLING_EN
#else
define NOTCH_HOWLING_EN  0
define PITCHSHIFT_HOWLING_EN 0
define HOWLING_SEL  -1
#endif

-
** 变声功能使能
#ifdef HAS_VOICE_PITCH_EFFECT
define VO_PITCH_EN  1
#else
define VO_PITCH_EN  0
#endif

-
** 动物变声功能使能
#ifdef HAS_VOICE_CHANGER_EFFECT
define VO_CHANGER_EN  1
#else
define VO_CHANGER_EN  0
#endif

-
** ECHO混响功能使能
#ifdef HAS_ECHO_EFFECT
define ECHO_EN   1
#else
define ECHO_EN   0
#endif

-
** AUDIO_HW_EQ功能使能
#ifdef HAS_AUDIO_HW_EQ_EFFECT
define AUDIO_HW_EQ_EN	1
#endif

-
** PCM_SW_EQ功能使能
#ifdef HAS_PCM_SW_EQ_EFFECT
define PCM_SW_EQ_EN		1
#endif

-
** A/F1A格式的解码文件需要同时判断后缀名确认采样率
if ( DECODER_A_EN || DECODER_F1A_EN  )
define DECODE_SR_IS_NEED_JUDIGMENT 1
else
define DECODE_SR_IS_NEED_JUDIGMENT 0
endif

-
** 文件系统
#ifdef HAS_FATFS_EN
define FATFS_EN 1
#else
define FATFS_EN 0
#endif

#ifdef HAS_NORFS_EN
define NORFS_EN 1
#else
define NORFS_EN 0
#endif

-
** 手机APP应用升级
define CONFIG_APP_OTA_EN 0

-
** APP应用使能
#ifdef HAS_MUSIC_MODE
define MUSIC_MODE_EN  1  -- mbox音乐应用模式
#else
define MUSIC_MODE_EN  0  -- mbox音乐应用模式
#endif

#ifdef HAS_RECORD_MODE
define RECORD_MODE_EN  1  -- 录音应用模式
#else
define RECORD_MODE_EN  0  -- 录音应用模式
#endif

#ifdef HAS_LINEIN_MODE
define LINEIN_MODE_EN  1 -- Linein应用模式
#else
define LINEIN_MODE_EN  0 -- Linein应用模式
#endif

#ifdef HAS_SIMPLE_DEC_MODE
define SIMPLE_DEC_EN  1  -- 简单解码应用模式
#else
define SIMPLE_DEC_EN  0  -- 简单解码应用模式
#endif

#ifdef HAS_LOUDSPEAKER_MODE
define LOUDSPEAKER_EN  1  -- 扩音应用模式
#else
define LOUDSPEAKER_EN  0  -- 扩音应用模式
#endif

#ifdef HAS_RTC_MODE
define RTC_EN  0  -- RTC模式
#else
define RTC_EN  0  -- RTC模式
#endif

#ifdef HAS_RF_RADIO_MODE
define RF_RADIO_EN  1  -- 对讲机模式
#else
define RF_RADIO_EN  0  -- 对讲机模式
#endif

define RF_REMOTECONTROL_MODE_EN  0  -- 遥控器模式
-
-
else
-
-
define DECODER_A_EN  0
define DECODER_MP3_ST_EN 0
define DECODER_WAV_EN 0
define DECODER_F1A_EN  0
define MAX_F1A_CHANNEL 0
define DECODER_UMP3_EN 0
define DECODER_MIDI_EN 0
define DECODER_MIDI_KEYBOARD_EN 0
define ENCODER_MP3_STEREO     0
define ENCODER_MP3_EN     0
define ENCODER_UMP3_EN  0
define ENCODER_A_EN  0
define HAS_MIO_EN  0
define HAS_SPEED_EN  0
define HAS_SONG_SPEED_EN  0
define NOTCH_HOWLING_EN  0
define PITCHSHIFT_HOWLING_EN 0
define HOWLING_SEL  -1
define VO_PITCH_EN  0
define VO_CHANGER_EN  0
define ECHO_EN   0
define PCM_SW_EQ_EN		0
define DECODE_SR_IS_NEED_JUDIGMENT 0
define FATFS_EN 0
define NORFS_EN 0

-
** opus格式压缩
#ifdef HAS_OPUS_ENCODER
define ENCODER_OPUS_EN  1
#else
define ENCODER_OPUS_EN  0
#endif

-
** ADPCM-IMA格式解码
#ifdef HAS_IMA_DECODER
define DECODER_IMA_EN  1
#else
define DECODER_IMA_EN  0
#endif

-
** SPEEX格式解码
#ifdef HAS_SPEEX_DECODER
define DECODER_SPEEX_EN  1
#else
define DECODER_SPEEX_EN  0
#endif

-
** SBC格式解码
#ifdef HAS_SBC_DECODER
define DECODER_SBC_EN  1
#else
define DECODER_SBC_EN  0
#endif

-
** OPUS格式解码
#ifdef HAS_OPUS_DECODER
define DECODER_OPUS_EN 1
#else
define DECODER_OPUS_EN 0
#endif

-
** SPEEX格式压缩
#ifdef HAS_SPEEX_ENCODER
define ENCODER_SPEEX_EN  1
#else
define ENCODER_SPEEX_EN  0
#endif

-
** ADPCM-IMA格式压缩
#ifdef HAS_IMA_ENCODER
define ENCODER_IMA_EN  1
#else
define ENCODER_IMA_EN  0
#endif

-
** SBC格式压缩
#ifdef HAS_SBC_ENCODER
define ENCODER_SBC_EN  1
#else
define ENCODER_SBC_EN  0
#endif

-
** 手机APP应用升级
#ifdef HAS_CONFIG_APP_OTA_EN
define CONFIG_APP_OTA_EN 1
#else
define CONFIG_APP_OTA_EN 0
#endif

-
** APP应用使能
define MUSIC_MODE_EN  0  -- mbox音乐应用模式
define RECORD_MODE_EN  0  -- 录音应用模式
define LINEIN_MODE_EN  0 -- Linein应用模式
define SIMPLE_DEC_EN  0  -- 简单解码应用模式
define LOUDSPEAKER_EN  0  -- 扩音应用模式
define RTC_EN  0  -- RTC模式
define RF_RADIO_EN  0  -- 对讲机模式
#ifdef HAS_RF_REMOTECONTROL_MODE
define RF_REMOTECONTROL_MODE_EN  1  -- 遥控器模式
#else
define RF_REMOTECONTROL_MODE_EN  0  -- 遥控器模式
#endif
-
endif

-
** 无缝循环使能
#ifdef HAS_DECODER_LOOP_EN
define DECODER_LOOP_EN 1
#else
define DECODER_LOOP_EN 0
#endif

-
** 定时任务注册功能使能
#ifdef HAS_SYS_TIMER_EN
define SYS_TIMER_EN   1
#else
define SYS_TIMER_EN   0
#endif

-
** 蓝牙BLE功能使能
#ifdef HAS_BLE_EN
define BLE_EN   1
#else
define BLE_EN   0
#endif

-
** 升级功能使能
#ifdef HAS_UPDATE_V2_EN
define UPDATE_V2_EN   1
#else
define UPDATE_V2_EN 0
#endif

-
** 充电仓/蓝牙测试盒单线串口升级
#ifdef HAS_TESTBOX_UART_UPDATE_EN
define TESTBOX_UART_UPDATE_EN 0
#else
define TESTBOX_UART_UPDATE_EN 0
#endif

-
** 测试盒蓝牙升级
#ifdef HAS_TESTBOX_BT_UPDATE_EN
define TESTBOX_BT_UPDATE_EN 1
#else
define TESTBOX_BT_UPDATE_EN 0
#endif

-
** SD卡设备升级
#ifdef HAS_SD_UPDATE_EN
define SD_UPDATE_EN 1
#else
define SD_UPDATE_EN 0
#endif

-
** U盘设备升级
#ifdef HAS_UDISK_UPDATE_EN
define UDISK_UPDATE_EN 1
#else
define UDISK_UPDATE_EN 0
#endif

-
-
endif
