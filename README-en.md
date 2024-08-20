# fw-AW30N
About Firmware for Generic MCU SDK（AW30N series）, Support AW30N

[tag download]:https://github.com/Jieli-Tech/AW30N/tags
[tag_badgen]:https://img.shields.io/github/v/tag/Jieli-Tech/AW30N?style=plastic&logo=bluetooth&label=tag&labelColor=ffffff&color=informational

# fw-AW30N_SDK   [![tag][tag_badgen]][tag download]

[中文](./README.md) | EN

firmware for Generic MCU SDK（AW30N series）

This repository contains the Jieli source code, aims at helping the developers for the BLE & toy & small music speaker & generic MCU applications.
It must be combined with lib.a and the repositories that use the same
naming convention to build the provided samples and to use the additional
subsystems and libraries.

Getting Started
------------

Welcome to JL open source! See the `Introduction to SDK` for a high-level overview,
and the documentation's `Getting Started Guide` to start developing.

Toolchain
------------

How to get the `JL Toolchain` and setup the build enviroment,see below

* Complie Tool ：install the JL complie tool to setup the build enviroment, [download link](https://pan.baidu.com/s/1f5pK7ZaBNnvbflD-7R22zA) code: `ukgx`
* Compiler invitation code: 4VlZaQCq-lImlZX2u-GBeCs501-ektNxDGu

* USB updater : program flash tool to download the `hex` file to the target board, please accquire the tool form the [link](https://item.taobao.com/item.htm?spm=a1z10.1-c-s.w4004-22883854875.5.504d246bXKwyeH&id=620295020803) and check the related configuration and [document](.doc/stuff/ISD_CONFIG.INI配置文件说明.pdf)

* Jieli BLE Profile production tool: [Link](https://gitee.com/Jieli-Tech/fw-AC63_BT_SDK/tree/master/sdk_tools/BLE%20Profile%E5%B7%A5%E5%85%B7 )

* Jieli BLE OTA upgrade tool (not supported yet): [Link](https://gitee.com/Jieli-Tech/fw-AC63_BT_SDK/tree/master/sdk_tools/BLE%20OTA%E5%8D%87%E7%BA%A7%E5%B7%A5%E5%85%B7)

* Jieli BLE Dongle OTA upgrade tool (not supported yet): [Link](https://gitee.com/Jieli-Tech/fw-AC63_BT_SDK/tree/master/sdk_tools/USB%20Dongle%20OTA%E5%8D%87%E7%BA%A7%E5%B7%A5%E5%85%B7)

Documentation
------------

* Chipset brief : [SoC datasheet](./doc)

* Product Select Guide : [SoC Select Guide.pdf](doc/杰理科技AW30N系列芯片选型表_20240816.pdf)

* Chip Guide : [SoC Select Guide.pdf](doc/AW30N_芯片手册_V1.1.pdf)

* SDK Version: [SDK History](doc/AW30N_SDK_发布版本信息.pdf)

* SDK introduction : [SDK quick start guide](./doc/AW30N_SDK手册_V1.6.pdf)

* Hardware Design Guide : [Hardware Design Guide](./doc/AW30N硬件设计指南V1.2.pdf)

* SDK Online documentation : [SDK Online documentation](https://doc.zh-jieli.com/AW30/zh-cn/master/index.html)

* SDK architure : [SDK module architure ](./doc/)

* Video resource: [Video resource](https://space.bilibili.com/3493277347088769/dynamic)

* FAE support document: [FAE support](https://gitee.com/jieli-tech_fae/fw-jl)

Build
-------------
Select a project to build. The following folders contains buildable projects:

* Voice toys &ble applications : ./sdk/AW30N_mbox_flash, usage:


Comming Soon：
-------------

SDK support Codeblock to build to project,make sure you already setup the enviroment

* Codeblock build : enter the project directory and find the `.cbp`,double click and build.

* Makefile build : `apps/app_cfg` select the target you want to build,double click the `make_prompt` and excute `make`

  `before build the project make sure the USB updater is connect and enter the update mode correctly`


Hardware
-------------

* EV Board ：(https://shop321455197.taobao.com/?spm=a230r.7195193.1997079397.2.2a6d391d3n5udo)

* Production Tool : massive prodution and program the SoC, please accquire the tool from the [link](https://item.taobao.com/item.htm?spm=a1z10.1-c-s.w4004-22883854875.8.504d246bXKwyeH&id=620941819219) and check the releated [doc](./doc/stuff/烧写器使用说明文档.pdf)

Packaging, audio file conversion, midi and other general audio tools
-------------

* [download link](https://pan.baidu.com/s/1ajzBF4BFeiRFpDF558ER9w#list/path=%2F) code: `3jey`
  
SDK function
-------------
* Supports single-mode 5.4 version of Bluetooth BLE;
* Supports complete GATT service functions and simple GATT service functions
* The complete GATT service is based on the standard GATT protocol and has a complete GATT profile;
* Simple GATT service is tailored according to the standard GATT protocol and supports simple data sending and receiving;
* Added BLE slave remote control application, BLE Dongle host application, and BLE walkie-talkie application examples;
* When the BLE slave remote control is not connected, the broadcast power consumption is 290uA+, and when connected, the standby power consumption is 130uA+;
* Support U disk/SD card device upgrade;
* Support test box serial port upgrade;
* Support test box Bluetooth upgrade;
* Support mobile phone Bluetooth OTA upgrade
* Support mobile phone USB upgrade
* Supports decoding, playback and encoding recording of system FLASH, resource FLASH, SDMMC, U disk and other devices, and supports up to three simultaneous decodings;
* Supports seven decoding playbacks of a/b/e, ump3, f1a/f1b/f1c/f1x, midi, standard mp3, wav, and opus;
* Supports four encoding recordings of standard mp2, a, ump2, and opus;
* Supports variable speed and pitch, echo, pitch change, howling suppression, PCM_EQ and other sound effects;
* Support decoding MIO function;
* Supports traditional toy application functions, including decoding USB slave, Linein, amplification, recording, and other applications;
* AUDIO_DAC supports mono single-ended output and 12 sampling rates including 8K~96k;
* AUDIO_APA (Class-D direct drive speaker) supports mono differential output and supports 3 sampling rates including 32K~48k;
* AUDIO_ADC supports mono single-ended/differential input and supports 9 sampling rates including 8K~48k;
* Support hardware resampling;
* Supports SOFT OFF shutdown and POWER DOWN hibernation, with shutdown power consumption of 2uA+ and hibernation power consumption of 61uA+, which will be greatly optimized in the future;

MCU information
-------------
* RISC / 240MHz /64K+16K
* flash 
* UART * 2 / IIC / SPI * 3 
* 12bit ADC
* 16bit audio adc
* 16bit audio dac
* class-d（APA）/ 0.5W
* ALink
* SD IO
* IR
* 16bit timer * 4
* usb 2.0 / full speed
* mcpwm * 3
* ble

Community
--------------

* [Dingtalk Group](./doc/stuff/dingtalk.jpg)

AW30N_SDK zhīchí AW30N xìliè xīnpiàn kāifā. AW30N xìliè xīnpiàn zhīchíle tōngyòng MCU chángjiàn yìngyòng, kěyǐ zuòwéi kāifā, pínggū, yàngpǐn, shènzhì liàng chǎn shǐyòng, duìyìng SDK bǎnběn jiàn Release
展开

Disclaimer
------------​

AW30N_SDK supports AW30N series chip development.
The AW30N series chips support common MCU applications and can be used for development, evaluation, samples, and even mass production. For the corresponding SDK version, see Release