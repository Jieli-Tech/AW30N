
# make 编译并下载
# make VERBOSE=1 显示编译详细过程
# make clean 清除编译临时文件
#
# 注意： Linux 下编译方式：
#     1. 从 http://pkgman.jieliapp.com/doc/all 处找到下载链接
#     2. 下载后，解压到 /opt/jieli 目录下，保证
#       /opt/jieli/common/bin/clang 存在（注意目录层次）
#     3. 确认 ulimit -n 的结果足够大（建议大于8096），否则链接可能会因为打开文件太多而失败
#       可以通过 ulimit -n 8096 来设置一个较大的值
#

# 工具路径设置
ifeq ($(OS), Windows_NT)
# Windows 下工具链位置
TOOL_DIR := C:/JL/pi32/bin
CC    := clang.exe
CXX   := clang.exe
LD    := lto-wrapper.exe
AR    := llvm-ar.exe
MKDIR := mkdir_win -p
RM    := rm -rf

SYS_LIB_DIR := C:/JL/pi32/libc
SYS_INC_DIR := C:/JL/pi32/include/libc
EXT_CFLAGS  := # Windows 下不需要 -D__SHELL__
export PATH:=$(TOOL_DIR);$(PATH)

## 后处理脚本
FIXBAT          := tools\utils\fixbat.exe # 用于处理 utf8->gbk 编码问题
POST_SCRIPT     := apps/app/post_build/bd49/download.bat
RUN_POST_SCRIPT := apps\app\post_build\bd49\download.bat
else
# Linux 下工具链位置
TOOL_DIR := /opt/jieli/pi32/bin
CC    := clang
CXX   := clang
LD    := lto-wrapper
AR    := lto-ar
MKDIR := mkdir -p
RM    := rm -rf
export OBJDUMP := $(TOOL_DIR)/objdump
export OBJCOPY := $(TOOL_DIR)/objcopy
export OBJSIZEDUMP := $(TOOL_DIR)/objsizedump

SYS_LIB_DIR := $(TOOL_DIR)/../lib
SYS_INC_DIR := $(TOOL_DIR)/../include
EXT_CFLAGS  := -D__SHELL__ # Linux 下需要这个保证正确处理 download.c
export PATH:=$(TOOL_DIR):$(PATH)

## 后处理脚本
FIXBAT          := touch # Linux下不需要处理 bat 编码问题
POST_SCRIPT     := apps/app/post_build/bd49/download.sh
RUN_POST_SCRIPT := bash $(POST_SCRIPT)
endif

CC  := $(TOOL_DIR)/$(CC)
CXX := $(TOOL_DIR)/$(CXX)
LD  := $(TOOL_DIR)/$(LD)
AR  := $(TOOL_DIR)/$(AR)
# 输出文件设置
OUT_ELF   := apps/app/post_build/bd49/sdk.elf
OBJ_FILE  := $(OUT_ELF).objs.txt
# 编译路径设置
BUILD_DIR := objs

# 编译参数设置
CFLAGS := \
	-target pi32 \
	-integrated-as \
	-fno-builtin \
	-mllvm -pi32-memreg-opt \
	-mllvm -pi32-mem-offset-adj-opt \
	-mllvm -pi32-const-spill \
	-mllvm -pi32-enable-jz \
	-mllvm -pi32-tailcall-opt \
	-mllvm -inline-threshold=5 \
	-mllvm -pi32-enable-itblock=1 \
	-Oz \
	-flto \
	-integrated-as \
	-g \
	-O0 \
	-flto \
	-Os \
	-Wcast-align \
	-w \
	-fallow-pointer-null \
	-Wincompatible-pointer-types \
	-Wundef \
	-nostrictpi32 \
	-fprefer-gnu-section \
	-Werror=implicit-function-declaration \
	-Werror=return-type \
	-Werror=undef \
	-Wno-format \


# C++额外的编译参数
CXXFLAGS :=


# 宏定义
DEFINES := \
	-D__FPGA=0 \
	-DCONFIG_CPU_BD49=1 \
	-DAPP_BT_BLE=1 \
	-DSUPPORT_MS_EXTENSIONS \
	-DD_MASKROM_CODE=0 \
	-DD_IS_FLASH_SYSTEM \
	-DD_SFC_DEVICE_EN \
	-DHAS_SRC_EN=1 \
	-DHAS_MIO_PLAYER \
	-DHAS_MP3_ST_DECODER \
	-DHAS_WAV_DECODER \
	-DHAS_F1A_DECODER \
	-DHAS_MAX_F1A_NUMBER=2 \
	-DHAS_UMP3_DECODER \
	-DHAS_A_DECODER \
	-DHAS_IMA_DECODER \
	-DHAS_MIDI_DECODER \
	-DHAS_MIDI_KEYBOARD_DECODER \
	-DHAS_OPUS_DECODER \
	-DHAS_SPEEX_DECODER \
	-DHAS_SBC_DECODER \
	-DHAS_MP3_ENCODER \
	-DMP3_ENCODER_SUPPORT_CH=1 \
	-DHAS_UMP3_ENCODER \
	-DHAS_A_ENCODER \
	-DHAS_OPUS_ENCODER \
	-DHAS_SPEEX_ENCODER \
	-DHAS_IMA_ENCODER \
	-DHAS_SBC_ENCODER \
	-DHAS_SPEED_EFFECT \
	-DHAS_SONG_SPEED_EFFECT \
	-DHAS_HOWLING_EFFECT \
	-DHAS_ECHO_EFFECT \
	-DHAS_VOICE_PITCH_EFFECT \
	-DHAS_VOICE_CHANGER_EFFECT \
	-DHAS_PCM_SW_EQ_EFFECT \
	-DHAS_USB_EN=1 \
	-DHAS_SDMMC_EN=1 \
	-DHAS_EXTFLASH_EN=1 \
	-DHAS_BLE_EN \
	-DHAS_VFS_EN=1 \
	-DHAS_NORFS_EN=1 \
	-DHAS_SYDFS_EN=1 \
	-DHAS_FATFS_EN=1 \
	-DHAS_SYS_TIMER_EN=1 \
	-DHAS_UPDATE_V1_EN=0 \
	-DHAS_UPDATE_V2_EN=1 \
	-DHAS_SD_UPDATE_EN=1 \
	-DHAS_UDISK_UPDATE_EN=1 \
	-DHAS_TESTBOX_BT_UPDATE_EN=1 \
	-DHAS_TESTBOX_UART_UPDATE_EN=1 \
	-DHAS_CONFIG_APP_OTA_EN=1 \
	-DHAS_LINEIN_MODE \
	-DHAS_SIMPLE_DEC_MODE \
	-DHAS_RECORD_MODE \
	-DHAS_MUSIC_MODE \
	-DHAS_USB_SLAVE_MODE \
	-DHAS_LOUDSPEAKER_MODE \
	-DHAS_RTC_MODE \
	-DHAS_RF_RADIO_MODE \
	-DHAS_RF_REMOTECONTROL_MODE \
	-DHAS_VM_EN \
	-DHAS_NEW_VM_MODE \
	-DNOFLOAT \


DEFINES += $(EXT_CFLAGS) # 额外的一些定义

# 头文件搜索路径
INCLUDES := \
	-Iapps/include_lib/fs/sydf \
	-Iapps/include_lib/fs/nor_fs \
	-Iapps/include_lib/fs/fat \
	-Iapps/app/bsp/common/decoder/mio \
	-Iapps/include_lib/encoder \
	-Iapps/include_lib/encoder/list \
	-Iapps/app/src \
	-Iapps/app/post_build/bd49 \
	-Iapps/app/bsp \
	-Iapps/app/bsp/common \
	-Iapps/app/bsp/common/key \
	-Iapps/app/bsp/common/power_manage \
	-Iapps/app/bsp/common/usb/usr \
	-Iapps/app/bsp/lib \
	-Iapps/app/bsp/cpu/bd49 \
	-Iapps/app/bsp/device \
	-Iapps/app/bsp/rf \
	-Iapps/app/bsp/start \
	-Iapps/app/bsp/start/bd49 \
	-Iapps/app/src/mbox_flash \
	-Iapps/app/src/mbox_flash/bd49 \
	-Iapps/include_lib \
	-Iapps/include_lib/common \
	-Iapps/include_lib/audio \
	-Iapps/include_lib/audio_eq \
	-Iapps/include_lib/cpu/bd49 \
	-Iapps/include_lib/cpu/bd49/asm \
	-Iapps/include_lib/cpu/bd49/power \
	-Iapps/include_lib/cpu \
	-Iapps/include_lib/msg \
	-Iapps/include_lib/fs \
	-Iapps/include_lib/dev_mg \
	-Iapps/include_lib/decoder \
	-Iapps/include_lib/decoder/list \
	-Iapps/include_lib/sound_mge \
	-Iapps/include_lib/device \
	-Iapps/include_lib/device/sdmmc \
	-Iapps/include_lib/device/usb \
	-Iapps/include_lib/device/usb/device \
	-Iapps/include_lib/device/usb/host \
	-Iapps/include_lib/device/norflash \
	-Iapps/include_lib/update/code_v2 \
	-Iapps/include_lib/config \
	-Iapps/include_lib/sound_effect_list \
	-Iapps/include_lib/pcm_eq \
	-Iapps/app/src/mbox_flash/common \
	-Iapps/app/src/mbox_flash/common/ui \
	-Iapps/app/src/mbox_flash/music \
	-Iapps/app/src/mbox_flash/usb_slave \
	-Iapps/app/src/mbox_flash/linein \
	-Iapps/app/src/mbox_flash/record \
	-Iapps/app/src/mbox_flash/simple_decode \
	-Iapps/app/src/mbox_flash/midi_dec \
	-Iapps/app/src/mbox_flash/midi_keyboard \
	-Iapps/app/src/mbox_flash/loudspeaker \
	-Iapps/app/src/mbox_flash/rtc \
	-Iapps/app/src/mbox_flash/softoff_app \
	-Iapps/app/src/mbox_flash/rf_radio \
	-Iapps/app/src/mbox_flash/remotecontrol \
	-Iapps/include_lib/liba/ARCH/pi32_lto/header \
	-Iapps/app/bsp/common/bt_common \
	-Iapps/app/bsp/common/bt_common/third_party_profile/jieli/gatt_common \
	-Iapps/app/bsp/common/audio_rf_trans \
	-Iapps/include_lib/bt_controller_include \
	-Iapps/include_lib/bt_controller_include/port/bd49 \
	-Iapps/include_lib/bt_include \
	-Iapps/lib/bt_controller \
	-Iapps/lib/bt_protocol \
	-Iapps/app/bsp/common/usb \
	-Iapps/app/bsp/common/usb/device \
	-Iapps/app/bsp/common/usb/host \
	-Iapps/include_lib/update \
	-Iapps/app/bsp/common/third_party_profile/jieli/JL_rcsp/rcsp_update \
	-Iapps/app/bsp/common/third_party_profile/jieli/JL_rcsp/rcsp_hid \
	-Iapps/app/bsp/common/third_party_profile/jieli/JL_rcsp \
	-Iapps/app/bsp/common/third_party_profile/common \
	-Iapps/include_lib/bt_include/third_party/rcsp \
	-Iapps/app/bsp/common/third_party_profile/jieli/gatt_common \
	-Iapps/app/bsp/common/bt_common/ble_test \
	-Iapps/include_lib/bt_controller_include/ble \
	-Iapps/include_lib/bt_controller_include/adapter/include/common \
	-Iapps/app/bsp/common/bt_common/hid \
	-Iapps/app/bsp/common/bt_common/hid/include \
	-Iapps/app/bsp/common/bt_common/spp_and_le \
	-Iapps/app/bsp/common/bt_common/spp_and_le/examples/dongle \
	-Iapps/app/bsp/common/bt_common/spp_and_le/include \
	-I$(SYS_INC_DIR) \


# 需要编译的 .c 文件
c_SRC_FILES := \
	apps/app/bsp/common/audio/audio_adc_api.c \
	apps/app/bsp/common/audio/audio_dac_fade.c \
	apps/app/bsp/common/audio/fill_audac.c \
	apps/app/bsp/common/audio/sound_kick.c \
	apps/app/bsp/common/audio_rf_trans/audio2rf_send.c \
	apps/app/bsp/common/audio_rf_trans/audio_rf_mge.c \
	apps/app/bsp/common/audio_rf_trans/rf2audio_recv.c \
	apps/app/bsp/common/audio_rf_trans/rf_pa_port.c \
	apps/app/bsp/common/audio_rf_trans/rf_send_queue.c \
	apps/app/bsp/common/audio_rf_trans/trans_packet.c \
	apps/app/bsp/common/audio_rf_trans/trans_unpacket.c \
	apps/app/bsp/common/bt_common/ble_test/ble_master.c \
	apps/app/bsp/common/bt_common/ble_test/ble_slave.c \
	apps/app/bsp/common/bt_common/ble_test/bt_ble.c \
	apps/app/bsp/common/bt_common/ble_test/lib_btctrler_config.c \
	apps/app/bsp/common/bt_common/ble_test/lib_btstack_config.c \
	apps/app/bsp/common/bt_common/ble_test/lib_profile_config.c \
	apps/app/bsp/common/bt_common/hid/app_main.c \
	apps/app/bsp/common/bt_common/hid/config/lib_btctrler_config.c \
	apps/app/bsp/common/bt_common/hid/config/lib_btstack_config.c \
	apps/app/bsp/common/bt_common/hid/config/lib_driver_config.c \
	apps/app/bsp/common/bt_common/hid/config/lib_profile_config.c \
	apps/app/bsp/common/bt_common/hid/config/lib_system_config.c \
	apps/app/bsp/common/bt_common/hid/config/lib_update_config.c \
	apps/app/bsp/common/bt_common/hid/config/log_config.c \
	apps/app/bsp/common/bt_common/hid/examples/keyboard/app_hid_comm.c \
	apps/app/bsp/common/bt_common/hid/modules/bt/app_comm_ble.c \
	apps/app/bsp/common/bt_common/hid/modules/bt/ble_hogp.c \
	apps/app/bsp/common/bt_common/hid/modules/user_cfg.c \
	apps/app/bsp/common/bt_common/hid/version.c \
	apps/app/bsp/common/bt_common/spp_and_le/app_main.c \
	apps/app/bsp/common/bt_common/spp_and_le/config/lib_btctrler_config.c \
	apps/app/bsp/common/bt_common/spp_and_le/config/lib_btstack_config.c \
	apps/app/bsp/common/bt_common/spp_and_le/config/lib_driver_config.c \
	apps/app/bsp/common/bt_common/spp_and_le/config/lib_profile_config.c \
	apps/app/bsp/common/bt_common/spp_and_le/config/lib_system_config.c \
	apps/app/bsp/common/bt_common/spp_and_le/config/lib_update_config.c \
	apps/app/bsp/common/bt_common/spp_and_le/config/log_config.c \
	apps/app/bsp/common/bt_common/spp_and_le/examples/dongle/app_dongle_comm.c \
	apps/app/bsp/common/bt_common/spp_and_le/examples/dongle/ble_dg_central.c \
	apps/app/bsp/common/bt_common/spp_and_le/examples/dongle/ota_dg_central.c \
	apps/app/bsp/common/bt_common/spp_and_le/examples/dongle/usb_hid_devices.c \
	apps/app/bsp/common/bt_common/spp_and_le/examples/dongle/usb_second_hid_devices.c \
	apps/app/bsp/common/bt_common/spp_and_le/modules/bt/app_comm_ble.c \
	apps/app/bsp/common/bt_common/spp_and_le/modules/user_cfg.c \
	apps/app/bsp/common/bt_common/spp_and_le/version.c \
	apps/app/bsp/common/bt_common/vble_complete.c \
	apps/app/bsp/common/bt_common/vble_simple.c \
	apps/app/bsp/common/decoder/audio_codec_format.c \
	apps/app/bsp/common/decoder/decoder_api.c \
	apps/app/bsp/common/decoder/decoder_msg_tab.c \
	apps/app/bsp/common/decoder/decoder_point.c \
	apps/app/bsp/common/decoder/list/a_api.c \
	apps/app/bsp/common/decoder/list/f1a_api.c \
	apps/app/bsp/common/decoder/list/f1x_parsing.c \
	apps/app/bsp/common/decoder/list/ima_api.c \
	apps/app/bsp/common/decoder/list/midi_api.c \
	apps/app/bsp/common/decoder/list/midi_ctrl_api.c \
	apps/app/bsp/common/decoder/list/mp3_standard_api.c \
	apps/app/bsp/common/decoder/list/opus_api.c \
	apps/app/bsp/common/decoder/list/sbc_api.c \
	apps/app/bsp/common/decoder/list/speex_api.c \
	apps/app/bsp/common/decoder/list/ump3_api.c \
	apps/app/bsp/common/decoder/list/wav_api.c \
	apps/app/bsp/common/decoder/mio/mio_api.c \
	apps/app/bsp/common/decoder/mio/mio_phy.c \
	apps/app/bsp/common/decoder/mp_io.c \
	apps/app/bsp/common/encoder/encoder_api.c \
	apps/app/bsp/common/encoder/encoder_file.c \
	apps/app/bsp/common/encoder/encoder_stream.c \
	apps/app/bsp/common/encoder/list/a_encoder.c \
	apps/app/bsp/common/encoder/list/ima_encoder.c \
	apps/app/bsp/common/encoder/list/mp3_encoder.c \
	apps/app/bsp/common/encoder/list/opus_encoder.c \
	apps/app/bsp/common/encoder/list/sbc_encoder.c \
	apps/app/bsp/common/encoder/list/speex_encoder.c \
	apps/app/bsp/common/encoder/list/ump3_encoder.c \
	apps/app/bsp/common/fs/fat/fat_resource.c \
	apps/app/bsp/common/fs/nor_fs/nor_fs_resource.c \
	apps/app/bsp/common/fs/sydf/sydf_resource.c \
	apps/app/bsp/common/fs/vfs.c \
	apps/app/bsp/common/fs/vfs_fat.c \
	apps/app/bsp/common/fs/vfs_resource.c \
	apps/app/bsp/common/ir/ir_decoder.c \
	apps/app/bsp/common/ir/ir_encoder.c \
	apps/app/bsp/common/key/key.c \
	apps/app/bsp/common/key/key_drv_ad.c \
	apps/app/bsp/common/key/key_drv_io.c \
	apps/app/bsp/common/key/key_drv_matrix.c \
	apps/app/bsp/common/key/key_ir.c \
	apps/app/bsp/common/key/key_lptouch.c \
	apps/app/bsp/common/key/key_touch.c \
	apps/app/bsp/common/msg/custom_event.c \
	apps/app/bsp/common/msg/msg.c \
	apps/app/bsp/common/my_malloc.c \
	apps/app/bsp/common/norflash/norflash.c \
	apps/app/bsp/common/power_manage/app_power_mg.c \
	apps/app/bsp/common/sound_effect_list/echo_api.c \
	apps/app/bsp/common/sound_effect_list/notch_howling_api.c \
	apps/app/bsp/common/sound_effect_list/pcm_eq_api.c \
	apps/app/bsp/common/sound_effect_list/pitchshift_howling_api.c \
	apps/app/bsp/common/sound_effect_list/song_speed_api.c \
	apps/app/bsp/common/sound_effect_list/speed_api.c \
	apps/app/bsp/common/sound_effect_list/vo_pitch_api.c \
	apps/app/bsp/common/sound_effect_list/voicechanger_api.c \
	apps/app/bsp/common/speaker/speak_api.c \
	apps/app/bsp/common/sys_memory/sys_memory.c \
	apps/app/bsp/common/tab_read.c \
	apps/app/bsp/common/third_party_profile/common/custom_cfg.c \
	apps/app/bsp/common/third_party_profile/jieli/JL_rcsp/rcsp_bluetooth.c \
	apps/app/bsp/common/third_party_profile/jieli/JL_rcsp/rcsp_hid/rcsp_hid_inter.c \
	apps/app/bsp/common/third_party_profile/jieli/JL_rcsp/rcsp_msg.c \
	apps/app/bsp/common/third_party_profile/jieli/JL_rcsp/rcsp_update/rcsp_ch_loader_download.c \
	apps/app/bsp/common/third_party_profile/jieli/JL_rcsp/rcsp_update/rcsp_user_update.c \
	apps/app/bsp/common/third_party_profile/jieli/gatt_common/le_gatt_client.c \
	apps/app/bsp/common/third_party_profile/jieli/gatt_common/le_gatt_common.c \
	apps/app/bsp/common/third_party_profile/jieli/gatt_common/le_gatt_server.c \
	apps/app/bsp/common/update/dev_update.c \
	apps/app/bsp/common/update/testbox_uart_update.c \
	apps/app/bsp/common/update/testbox_update.c \
	apps/app/bsp/common/update/update.c \
	apps/app/bsp/common/usb/device/cdc.c \
	apps/app/bsp/common/usb/device/custom_hid.c \
	apps/app/bsp/common/usb/device/descriptor.c \
	apps/app/bsp/common/usb/device/hid.c \
	apps/app/bsp/common/usb/device/msd.c \
	apps/app/bsp/common/usb/device/task_pc.c \
	apps/app/bsp/common/usb/device/uac1.c \
	apps/app/bsp/common/usb/device/uac_stream.c \
	apps/app/bsp/common/usb/device/usb_device.c \
	apps/app/bsp/common/usb/device/usb_device_config.c \
	apps/app/bsp/common/usb/device/usb_suspend_resume.c \
	apps/app/bsp/common/usb/device/user_setup.c \
	apps/app/bsp/common/usb/host/usb_bulk_transfer.c \
	apps/app/bsp/common/usb/host/usb_ctrl_transfer.c \
	apps/app/bsp/common/usb/host/usb_host.c \
	apps/app/bsp/common/usb/host/usb_host_config.c \
	apps/app/bsp/common/usb/host/usb_storage.c \
	apps/app/bsp/common/usb/usb_test.c \
	apps/app/bsp/common/usb/usr/auadc_2_usbmic.c \
	apps/app/bsp/common/usb/usr/source_pcm.c \
	apps/app/bsp/common/usb/usr/uac_sync.c \
	apps/app/bsp/common/usb/usr/usb_audio_interface.c \
	apps/app/bsp/common/usb/usr/usb_mic_interface.c \
	apps/app/bsp/common/vm/nvm_api.c \
	apps/app/bsp/common/vm/vm_api.c \
	apps/app/bsp/cpu/bd49/adc_api.c \
	apps/app/bsp/cpu/bd49/apa_cpu.c \
	apps/app/bsp/cpu/bd49/audio_adc_cpu.c \
	apps/app/bsp/cpu/bd49/audio_dac_api.c \
	apps/app/bsp/cpu/bd49/audio_dac_cpu.c \
	apps/app/bsp/cpu/bd49/device.c \
	apps/app/bsp/cpu/bd49/gpcnt.c \
	apps/app/bsp/cpu/bd49/hw_timer.c \
	apps/app/bsp/cpu/bd49/hw_timer_capture.c \
	apps/app/bsp/cpu/bd49/hw_timer_pwm.c \
	apps/app/bsp/cpu/bd49/iic_api.c \
	apps/app/bsp/cpu/bd49/iic_soft.c \
	apps/app/bsp/cpu/bd49/lpctmu.c \
	apps/app/bsp/cpu/bd49/mcpwm.c \
	apps/app/bsp/cpu/bd49/my_print.c \
	apps/app/bsp/cpu/bd49/pl_cnt.c \
	apps/app/bsp/cpu/bd49/power/gpio_config.c \
	apps/app/bsp/cpu/bd49/power/power_app.c \
	apps/app/bsp/cpu/bd49/power/power_config.c \
	apps/app/bsp/cpu/bd49/power_api.c \
	apps/app/bsp/cpu/bd49/rtc_app.c \
	apps/app/bsp/cpu/bd49/sd.c \
	apps/app/bsp/cpu/bd49/spi.c \
	apps/app/bsp/cpu/bd49/tick_timer_sys.c \
	apps/app/bsp/cpu/bd49/uart.c \
	apps/app/bsp/device/device_app.c \
	apps/app/bsp/device/device_list.c \
	apps/app/bsp/rf/rf_soft.c \
	apps/app/bsp/start/bd49/main.c \
	apps/app/bsp/start/bsp_loop.c \
	apps/app/bsp/start/flash_init.c \
	apps/app/bsp/start/init.c \
	apps/app/src/mbox_flash/app.c \
	apps/app/src/mbox_flash/app_config.c \
	apps/app/src/mbox_flash/bd49/cpu_config.c \
	apps/app/src/mbox_flash/common/hot_msg.c \
	apps/app/src/mbox_flash/common/idle_check.c \
	apps/app/src/mbox_flash/common/pa_mute.c \
	apps/app/src/mbox_flash/common/simple_play_file.c \
	apps/app/src/mbox_flash/common/ui/led5x7_driver.c \
	apps/app/src/mbox_flash/common/ui/ui_api.c \
	apps/app/src/mbox_flash/common/ui/ui_common.c \
	apps/app/src/mbox_flash/init_app.c \
	apps/app/src/mbox_flash/linein/linein_mode.c \
	apps/app/src/mbox_flash/linein/linein_mode_key.c \
	apps/app/src/mbox_flash/loudspeaker/loudspk_mode.c \
	apps/app/src/mbox_flash/loudspeaker/loudspk_mode_key.c \
	apps/app/src/mbox_flash/midi_dec/midi_dec_mode.c \
	apps/app/src/mbox_flash/midi_dec/midi_dec_mode_key.c \
	apps/app/src/mbox_flash/midi_keyboard/midi_keyboard_mode.c \
	apps/app/src/mbox_flash/midi_keyboard/midi_keyboard_mode_key.c \
	apps/app/src/mbox_flash/music/break_point.c \
	apps/app/src/mbox_flash/music/music_device.c \
	apps/app/src/mbox_flash/music/music_key_table.c \
	apps/app/src/mbox_flash/music/music_play.c \
	apps/app/src/mbox_flash/music/play_file.c \
	apps/app/src/mbox_flash/record/enc_in_fatfs.c \
	apps/app/src/mbox_flash/record/enc_in_norfs.c \
	apps/app/src/mbox_flash/record/record_mode.c \
	apps/app/src/mbox_flash/record/record_mode_key.c \
	apps/app/src/mbox_flash/remotecontrol/dongle.c \
	apps/app/src/mbox_flash/remotecontrol/rc_app.c \
	apps/app/src/mbox_flash/remotecontrol/rc_key.c \
	apps/app/src/mbox_flash/rf_radio/rf_radio_app.c \
	apps/app/src/mbox_flash/rf_radio/rf_radio_key.c \
	apps/app/src/mbox_flash/rf_radio/rf_radio_receive.c \
	apps/app/src/mbox_flash/rf_radio/rf_radio_send.c \
	apps/app/src/mbox_flash/rtc/rtc_mode.c \
	apps/app/src/mbox_flash/rtc/rtc_mode_key.c \
	apps/app/src/mbox_flash/simple_decode/simple_decode.c \
	apps/app/src/mbox_flash/simple_decode/simple_decode_key.c \
	apps/app/src/mbox_flash/softoff_app/softoff_mode.c \
	apps/app/src/mbox_flash/usb_slave/usb_slave_mode.c \
	apps/app/src/mbox_flash/usb_slave/usb_slave_mode_key.c \


# 需要编译的 .S 文件
S_SRC_FILES :=


# 需要编译的 .s 文件
s_SRC_FILES :=


# 需要编译的 .cpp 文件
cpp_SRC_FILES :=


# 需要编译的 .cc 文件
cc_SRC_FILES :=


# 需要编译的 .cxx 文件
cxx_SRC_FILES :=


# 链接参数
LFLAGS := \
	--plugin-opt=-inline-threshold=5 \
	--plugin-opt=save-temps \
	--plugin-opt=-inline-normal-into-special-section=true \
	--plugin-opt=-dont-used-symbol-list=malloc,free,sprintf,printf,puts,putchar \
	--plugin-opt=-mattr=+no-constpool \
	--sort-common \
	--start-group \
	apps/include_lib/liba/ARCH/pi32_lto/mp3_dec_lib.a \
	apps/include_lib/liba/ARCH/pi32_lto/lib_wav_decoder.a \
	apps/include_lib/liba/ARCH/pi32_lto/lib_f1a_decode.a \
	apps/include_lib/liba/ARCH/pi32_lto/lib_a_decode.a \
	apps/include_lib/liba/ARCH/pi32_lto/lib_ima_stream_decode.a \
	apps/include_lib/liba/ARCH/pi32_lto/ump3_dec_lib.a \
	apps/include_lib/liba/ARCH/pi32_lto/opus_dec_lib.a \
	apps/include_lib/liba/ARCH/pi32_lto/lib_speex_decode_encode.a \
	apps/include_lib/liba/ARCH/pi32_lto/sbc_stddec_lib.a \
	apps/include_lib/liba/ARCH/pi32_lto/lib_midi_decode.a \
	apps/include_lib/liba/bd49/mbox_flash/encoder_mge_lib.a \
	apps/include_lib/liba/ARCH/pi32_lto/lib_mp2_code.a \
	apps/include_lib/liba/ARCH/pi32_lto/ump2_enc_lib.a \
	apps/include_lib/liba/ARCH/pi32_lto/lib_a_code.a \
	apps/include_lib/liba/ARCH/pi32_lto/opus_enc_lib.a \
	apps/include_lib/liba/ARCH/pi32_lto/lib_speex_decode_encode.a  \
	apps/include_lib/liba/ARCH/pi32_lto/lib_ima_stream_enc.a \
	apps/include_lib/liba/ARCH/pi32_lto/sbc_stdenc_lib.a \
	apps/include_lib/liba/ARCH/pi32_lto/lib_speed_pitch.a \
	apps/include_lib/liba/ARCH/pi32_lto/ps_song_lib.a \
	apps/include_lib/liba/ARCH/pi32_lto/howling.a \
	apps/include_lib/liba/ARCH/pi32_lto/lib_howling_fs.a \
	apps/include_lib/liba/ARCH/pi32_lto/lib_echo_cal.a \
	apps/include_lib/liba/ARCH/pi32_lto/lib_vopitch_cal.a \
	apps/include_lib/liba/ARCH/pi32_lto/lib_voiceChanger_va.a \
	apps/include_lib/liba/ARCH/pi32_lto/lib_pcm_eq.a \
	apps/include_lib/liba/bd49/mbox_flash/ae_pcm_eq_lib.a \
	apps/include_lib/liba/bd49/mbox_flash/cpu_lib.a \
	apps/include_lib/liba/bd49/mbox_flash/fs_lib.a \
	apps/include_lib/liba/bd49/mbox_flash/malloc_lib.a \
	apps/include_lib/liba/bd49/mbox_flash/audio_lib.a \
	apps/include_lib/liba/bd49/mbox_flash/decoder_mge_lib.a \
	apps/include_lib/liba/bd49/mbox_flash/sound_mge_lib.a \
	apps/include_lib/liba/bd49/mbox_flash/src_lib.a \
	apps/include_lib/liba/bd49/mbox_flash/sdmmc_lib.a \
	apps/include_lib/liba/bd49/mbox_flash/sys_timer_lib.a \
	apps/include_lib/liba/bd49/mbox_flash/periphy_lib.a \
	apps/include_lib/liba/bd49/mbox_flash/rtc_mge_lib.a \
	apps/include_lib/liba/bd49/mbox_flash/new_vm_lib.a \
	apps/include_lib/liba/bd49/mbox_flash/printf_lib.a \
	apps/include_lib/liba/bd49/mbox_flash/dev_mg_lib.a \
	apps/include_lib/liba/bd49/mbox_flash/iir_lib.a \
	apps/include_lib/liba/bd49/mbox_flash/agreement.a \
	apps/include_lib/liba/bd49/mbox_flash/update_lib.a \
	apps/include_lib/liba/bd49/mbox_flash/bt_controller_lib.a \
	apps/include_lib/liba/bd49/mbox_flash/bt_protocol_lib.a \
	--end-group \
	-Tapps/app/post_build/bd49/app.ld \
	-M=apps/app/post_build/bd49/app.map \
	--plugin-opt=-pi32-memreg-opt \
	--plugin-opt=-pi32-mem-offset-adj-opt \
	--plugin-opt=-pi32-const-spill \
	--plugin-opt=-pi32-enable-jz \
	--plugin-opt=-pi32-tailcall-opt \
	--plugin-opt=-inline-threshold=5 \
	--plugin-opt=-pi32-enable-itblock=1 \


LIBPATHS := \
	-L$(SYS_LIB_DIR) \


LIBS := \
	C:/JL/pi32/lib/libc.a \
	C:/JL/pi32/lib/libm.a \
	C:/JL/pi32/lib/libcompiler-rt.a \



c_OBJS    := $(c_SRC_FILES:%.c=%.c.o)
S_OBJS    := $(S_SRC_FILES:%.S=%.S.o)
s_OBJS    := $(s_SRC_FILES:%.s=%.s.o)
cpp_OBJS  := $(cpp_SRC_FILES:%.cpp=%.cpp.o)
cxx_OBJS  := $(cxx_SRC_FILES:%.cxx=%.cxx.o)
cc_OBJS   := $(cc_SRC_FILES:%.cc=%.cc.o)

OBJS      := $(c_OBJS) $(S_OBJS) $(s_OBJS) $(cpp_OBJS) $(cxx_OBJS) $(cc_OBJS)
DEP_FILES := $(OBJS:%.o=%.d)


OBJS      := $(addprefix $(BUILD_DIR)/, $(OBJS))
DEP_FILES := $(addprefix $(BUILD_DIR)/, $(DEP_FILES))


VERBOSE ?= 0
ifeq ($(VERBOSE), 1)
QUITE :=
else
QUITE := @
endif

# 一些旧的 make 不支持 file 函数，需要 make 的时候指定 LINK_AT=0 make
LINK_AT ?= 1

# 表示下面的不是一个文件的名字，无论是否存在 all, clean, pre_build 这样的文件
# 还是要执行命令
# see: https://www.gnu.org/software/make/manual/html_node/Phony-Targets.html
.PHONY: all clean pre_build

# 不要使用 make 预设置的规则
# see: https://www.gnu.org/software/make/manual/html_node/Suffix-Rules.html
.SUFFIXES:

all: pre_build $(OUT_ELF)
	$(info +POST-BUILD)
	$(QUITE) $(RUN_POST_SCRIPT) sdk

pre_build:
	$(info +PRE-BUILD)
	$(QUITE) $(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -D__LD__ -E -P apps/app/bsp/common/app_modules_h.c -o tmp_app2022.txt
	$(QUITE) $(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -D__LD__ -E -P apps/app/post_build/bd49/mbox_flash/app_ld.c -o apps/app/post_build/bd49/app.ld
	$(QUITE) $(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -D__LD__ -E -P apps/app/post_build/bd49/mbox_flash/download_bat.c -o $(POST_SCRIPT)
	$(QUITE) $(FIXBAT) $(POST_SCRIPT)
	$(QUITE) $(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -D__LD__ -E -P apps/app/post_build/bd49/mbox_flash/download_sh.c -o apps/app/post_build/bd49/download.sh
	$(QUITE) $(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -D__LD__ -E -P apps/app/post_build/bd49/mbox_flash/isd_config_ini.c -o apps/app/post_build/bd49/isd_config.ini

clean:
	$(QUITE) $(RM) $(OUT_ELF)
	$(QUITE) $(RM) $(BUILD_DIR)



ifeq ($(LINK_AT), 1)
$(OUT_ELF): $(OBJS)
	$(info +LINK $@)
	$(shell $(MKDIR) $(@D))
	$(file >$(OBJ_FILE), $(OBJS))
	$(QUITE) $(LD) -o $(OUT_ELF) @$(OBJ_FILE) $(LFLAGS) $(LIBPATHS) $(LIBS)
else
$(OUT_ELF): $(OBJS)
	$(info +LINK $@)
	$(shell $(MKDIR) $(@D))
	$(QUITE) $(LD) -o $(OUT_ELF) $(OBJS) $(LFLAGS) $(LIBPATHS) $(LIBS)
endif


$(BUILD_DIR)/%.c.o : %.c
	$(info +CC $<)
	$(QUITE) $(MKDIR) $(@D)
	$(QUITE) $(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -MM -MT $@ $< -o $(@:.o=.d)
	$(QUITE) $(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -c $< -o $@

$(BUILD_DIR)/%.S.o : %.S
	$(info +AS $<)
	$(QUITE) $(MKDIR) $(@D)
	$(QUITE) $(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -MM -MT $@ $< -o $(@:.o=.d)
	$(QUITE) $(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -c $< -o $@

$(BUILD_DIR)/%.s.o : %.s
	$(info +AS $<)
	$(QUITE) $(MKDIR) $(@D)
	$(QUITE) $(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -MM -MT $@ $< -o $(@:.o=.d)
	$(QUITE) $(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -c $< -o $@

$(BUILD_DIR)/%.cpp.o : %.cpp
	$(info +CXX $<)
	$(QUITE) $(MKDIR) $(@D)
	$(QUITE) $(CXX) $(CXXFLAGS) $(CFLAGS) $(DEFINES) $(INCLUDES) -MM -MT $@ $< -o $(@:.o=.d)
	$(QUITE) $(CXX) $(CXXFLAGS) $(CFLAGS) $(DEFINES) $(INCLUDES) -c $< -o $@

$(BUILD_DIR)/%.cxx.o : %.cxx
	$(info +CXX $<)
	$(QUITE) $(MKDIR) $(@D)
	$(QUITE) $(CXX) $(CXXFLAGS) $(CFLAGS) $(DEFINES) $(INCLUDES) -MM -MT $@ $< -o $(@:.o=.d)
	$(QUITE) $(CXX) $(CXXFLAGS) $(CFLAGS) $(DEFINES) $(INCLUDES) -c $< -o $@

$(BUILD_DIR)/%.cc.o : %.cc
	$(info +CXX $<)
	$(QUITE) $(MKDIR) $(@D)
	$(QUITE) $(CXX) $(CXXFLAGS) $(CFLAGS) $(DEFINES) $(INCLUDES) -MM -MT $@ $< -o $(@:.o=.d)
	$(QUITE) $(CXX) $(CXXFLAGS) $(CFLAGS) $(DEFINES) $(INCLUDES) -c $< -o $@

-include $(DEP_FILES)
