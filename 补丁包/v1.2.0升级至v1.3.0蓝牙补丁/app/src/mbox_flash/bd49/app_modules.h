#ifndef __APP_MODULES_H__
#define __APP_MODULES_H__
//  此文件在在服务器编译库时会自动生成，源文件位于apps/app/bsp/common/app_modules_h.c
//  作者: 刘杰
//  日期:2022年11月22日
//  设计目的: 用于在应用代码中控制各种算法模块的使用
//  注意事项：不要在库文件中包含


//  APP方案选择
#define RUN_APP_CUSTOM 1
#define RUN_APP_RC 0
#define RUN_APP_DONGLE 0

//  ble trans_data app select
#if (RUN_APP_CUSTOM)
#define TESTE_BLE_EN 1
#define TRANS_DATA_HID_EN 0
#define TRANS_DATA_SPPLE_EN 0

#elif (RUN_APP_RC)
#define TESTE_BLE_EN 0
#define TRANS_DATA_HID_EN 1
#define TRANS_DATA_SPPLE_EN 0

#elif (RUN_APP_DONGLE)
#define TESTE_BLE_EN 0
#define TRANS_DATA_HID_EN 0
#define TRANS_DATA_SPPLE_EN 1
#endif


#if RUN_APP_CUSTOM

//  全双工对讲机功能开关（ram使用比较紧张，需要关掉部分算法）
#define FULL_DUPLEX_RADIO 0

//  A格式解码
#define DECODER_A_EN 1

//  ADPCM-IMA格式解码
#define DECODER_IMA_EN 1

//  标准MP3格式解码
#define DECODER_MP3_ST_EN 1

//  WAV格式解码
#define DECODER_WAV_EN 1

//  F1A格式解码
#define DECODER_F1A_EN 1
#define MAX_F1A_CHANNEL 2

//  UMP3格式解码
#define DECODER_UMP3_EN 1

//  MIDI格式解码
#define DECODER_MIDI_EN 1

//  MIDI琴格式解码
#define DECODER_MIDI_KEYBOARD_EN 1

//  OPUS格式解码
#define DECODER_OPUS_EN 0

//  SPEEX格式解码
#define DECODER_SPEEX_EN 1

//  SBC格式解码
#define DECODER_SBC_EN 1

//  mp3格式压缩
#define ENCODER_MP3_EN 1
#define ENCODER_MP3_STEREO 0

//  ump3格式压缩
#define ENCODER_UMP3_EN 1

//  a格式压缩
#define ENCODER_A_EN 1

//  opus格式压缩
#define ENCODER_OPUS_EN 0

//  SPEEX格式压缩
#define ENCODER_SPEEX_EN 1

//  adpcm-ima格式压缩
#define ENCODER_IMA_EN 1

//  SBC格式压缩
#define ENCODER_SBC_EN 1

//  MIO功能使能
#define HAS_MIO_EN 0

//  实时SPEED功能(用于扩音)使能
#define HAS_SPEED_EN 1

//  歌曲SPEED功能(用于解码)使能
#define HAS_SONG_SPEED_EN 1

//  陷波/移频啸叫抑制使能(二选一)
#define NOTCH_HOWLING_EN 1
#define PITCHSHIFT_HOWLING_EN 2
#define HOWLING_SEL PITCHSHIFT_HOWLING_EN

//  变声功能使能
#define VO_PITCH_EN 1

//  动物变声功能使能
#define VO_CHANGER_EN 1

//  ECHO混响功能使能
#define ECHO_EN 1

//  AUDIO_HW_EQ功能使能

//  PCM_SW_EQ功能使能
#define PCM_SW_EQ_EN 1

//  A/F1A格式的解码文件需要同时判断后缀名确认采样率
#if ( DECODER_A_EN || DECODER_F1A_EN )
#define DECODE_SR_IS_NEED_JUDIGMENT 1
#else
#define DECODE_SR_IS_NEED_JUDIGMENT 0
#endif

//  文件系统
#define FATFS_EN 1
#define NORFS_EN 1

//  手机APP应用升级
#define CONFIG_APP_OTA_EN 0

//  APP应用使能
#define MUSIC_MODE_EN 1 //  mbox音乐应用模式
#define RECORD_MODE_EN 1 //  录音应用模式
#define LINEIN_MODE_EN 1 //  Linein应用模式
#define SIMPLE_DEC_EN 1 //  简单解码应用模式
#define LOUDSPEAKER_EN 1 //  扩音应用模式
#define RTC_EN 0 //  RTC模式
#define RF_RADIO_EN 1 //  对讲机模式
#define RF_REMOTECONTROL_MODE_EN 0 //  遥控器模式


#else


#define DECODER_A_EN 0
#define DECODER_MP3_ST_EN 0
#define DECODER_WAV_EN 0
#define DECODER_F1A_EN 0
#define MAX_F1A_CHANNEL 0
#define DECODER_UMP3_EN 0
#define DECODER_MIDI_EN 0
#define DECODER_MIDI_KEYBOARD_EN 0
#define ENCODER_MP3_STEREO 0
#define ENCODER_MP3_EN 0
#define ENCODER_UMP3_EN 0
#define ENCODER_A_EN 0
#define HAS_MIO_EN 0
#define HAS_SPEED_EN 0
#define HAS_SONG_SPEED_EN 0
#define NOTCH_HOWLING_EN 0
#define PITCHSHIFT_HOWLING_EN 0
#define HOWLING_SEL -1
#define VO_PITCH_EN 0
#define VO_CHANGER_EN 0
#define ECHO_EN 0
#define PCM_SW_EQ_EN 0
#define DECODE_SR_IS_NEED_JUDIGMENT 0
#define FATFS_EN 0
#define NORFS_EN 0

//  opus格式压缩
#define ENCODER_OPUS_EN 1

//  ADPCM-IMA格式解码
#define DECODER_IMA_EN 1

//  SPEEX格式解码
#define DECODER_SPEEX_EN 1

//  SBC格式解码
#define DECODER_SBC_EN 1

//  OPUS格式解码
#define DECODER_OPUS_EN 1

//  SPEEX格式压缩
#define ENCODER_SPEEX_EN 1

//  ADPCM-IMA格式压缩
#define ENCODER_IMA_EN 1

//  SBC格式压缩
#define ENCODER_SBC_EN 1

//  手机APP应用升级
#define CONFIG_APP_OTA_EN 1

//  APP应用使能
#define MUSIC_MODE_EN 0 //  mbox音乐应用模式
#define RECORD_MODE_EN 0 //  录音应用模式
#define LINEIN_MODE_EN 0 //  Linein应用模式
#define SIMPLE_DEC_EN 0 //  简单解码应用模式
#define LOUDSPEAKER_EN 0 //  扩音应用模式
#define RTC_EN 0 //  RTC模式
#define RF_RADIO_EN 0 //  对讲机模式
#define RF_REMOTECONTROL_MODE_EN 1 //  遥控器模式

#endif

//  无缝循环使能
#define DECODER_LOOP_EN 0

//  定时任务注册功能使能
#define SYS_TIMER_EN 1

//  蓝牙BLE功能使能
#define BLE_EN 1

//  升级功能使能
#define UPDATE_V2_EN 1

//  充电仓/蓝牙测试盒单线串口升级
#define TESTBOX_UART_UPDATE_EN 0

//  测试盒蓝牙升级
#define TESTBOX_BT_UPDATE_EN 1

//  SD卡设备升级
#define SD_UPDATE_EN 1

//  U盘设备升级
#define UDISK_UPDATE_EN 1


#endif
