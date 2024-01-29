# fw-AW30N
About Firmware for Generic MCU SDK（AW30N series）, Support AW30N

[tag download]:https://github.com/Jieli-Tech/AW30N/tags
[tag_badgen]:https://img.shields.io/github/v/tag/Jieli-Tech/AW30N?style=plastic&logo=bluetooth&label=tag&labelColor=ffffff&color=informational

# fw-AW30N_SDK   [![tag][tag_badgen]][tag download]

中文 | [EN](./README-en.md)

AW30N 系列通用MCU SDK 固件程序

本仓库包含SDK release 版本代码，线下线上支持同步发布，支持BLE、玩具类、小音箱类产品和通用MCU类应用二次开发.

本工程提供的例子，需要结合对应命名规则的库文件(lib.a) 和对应的子仓库进行编译.


快速开始
------------

欢迎使用杰理开源项目，在开始进入项目之前，请详细阅读SDK 介绍文档，
从而获得对杰理系列芯片和SDK 的大概认识，并且可以通过快速开始介绍来进行开发.

工具链
------------

关于如何获取`杰理工具链` 和 如何进行环境搭建，请阅读以下内容：

* 编译工具 ：请安装杰理编译工具来搭建起编译环境, [下载链接](https://pan.baidu.com/s/1f5pK7ZaBNnvbflD-7R22zA) 提取码: `ukgx`
* 编译器邀请码：4VlZaQCq-lImlZX2u-GBeCs501-ektNxDGu

* USB 升级工具 : 在开发完成后，需要使用杰理烧写工具将对应的`hex`文件烧录到目标板，进行开发调试, 关于如何获取工具请进入申请 [链接](https://item.taobao.com/item.htm?spm=a1z10.1-c-s.w4004-22883854875.5.504d246bXKwyeH&id=620295020803) 并详细阅读对应的[文档](doc/stuff/usb%20updater.pdf)，以及相关下载脚本[配置](doc/stuff/ISD_CONFIG.INI配置文件说明.pdf)

* 杰理科技BLE Profile制作工具： [链接](https://gitee.com/Jieli-Tech/fw-AC63_BT_SDK/tree/master/sdk_tools/BLE%20Profile%E5%B7%A5%E5%85%B7)

* 杰理科技BLE OTA升级工具（暂未支持）： [链接](https://gitee.com/Jieli-Tech/fw-AC63_BT_SDK/tree/master/sdk_tools/BLE%20OTA%E5%8D%87%E7%BA%A7%E5%B7%A5%E5%85%B7)

* 杰理科技BLE Dongle OTA升级工具（暂未支持）： [链接](https://gitee.com/Jieli-Tech/fw-AC63_BT_SDK/tree/master/sdk_tools/USB%20Dongle%20OTA%E5%8D%87%E7%BA%A7%E5%B7%A5%E5%85%B7)



介绍文档
------------

* 芯片简介 : [SoC 数据手册扼要](./doc)

* 芯片选型号手册 : [SoC 选型手册](doc/杰理科技AW30N系列芯片选型表_20240124.pdf)

* SDK 版本信息 : [SDK 历史版本](doc/AW30N_SDK_发布版本信息.pdf)

* SDK 介绍文档 : [SDK 快速开始简介](./doc/AW30N_SDK手册_V1.2.pdf)

* SDK 在线文档 : [SDK 在线文档](https://doc.zh-jieli.com/AW30/zh-cn/master/index.html)

* SDK 结构文档 : [SDK 模块结构](./doc/)

* 视频资源 : [视频链接](https://space.bilibili.com/3493277347088769/dynamic)

* FAE 支持文档 : [FAE支持](https://gitee.com/jieli-tech_fae/fw-jl)

编译工程
-------------
请选择以下一个工程进行编译，下列目录包含了便于开发的工程文件：

* 语音&ble类应用 : ./sdk/AW30N_mbox_flash, 适用领域：

 即将发布：
------------

SDK 支持Codeblock编译环境，请确保编译前已经搭建好编译环境，

* Codeblock 编译 : 进入对应的工程目录并找到后缀为 `.cbp` 的文件, 双击打开便可进行编译.

* Makefile 编译 : `apps/app_cfg` 开始编译之前，需要先选择好目标应用并编辑保存, 请双击 `make_prompt` 并输入 `make`

  `在编译下载代码前，请确保USB 升级工具正确连接并且进入编程模式`


硬件环境
-------------

* 开发评估板 ：开发板申请入口[链接](https://shop321455197.taobao.com/?spm=a230r.7195193.1997079397.2.2a6d391d3n5udo)

* 生产烧写工具 : 为量产和裸片烧写而设计, 申请入口 [连接](https://item.taobao.com/item.htm?spm=a1z10.1-c-s.w4004-22883854875.8.504d246bXKwyeH&id=620941819219) 并仔细阅读相关 [文档](./doc/stuff/烧写器使用说明文档.pdf)
  
打包工具&音频文件转换工具
-------------

* [下载链接](https://pan.baidu.com/s/1ajzBF4BFeiRFpDF558ER9w#list/path=%2F) 提取码：`3jey` 

SDK主要功能
-------------
* 支持单模 5.4 版本的蓝牙 BLE；
* 支持完整 GATT 服务功能和简易 GATT 服务功能
* 完整 GATT 服务基于标准 GATT 协议，具有完整的 GATT profile；
* 简易 GATT 服务根据标准 GATT 协议进行裁切，支持简单数据收发；
* 新增 BLE 从机遥控器应用、BLE Dongle 主机应用、BLE 对讲机应用示例；
* BLE 从机遥控器未连接广播功耗为 290uA+，已连接待机功耗为 130uA+；
* 支持U 盘/SD 卡设备升级；
* 支持测试盒串口升级；
* 支持测试盒蓝牙升级；
* 支持手机蓝牙 OTA 升级
* 支持手机 USB 升级
* 支持系统 FLASH、资源 FLASH、SDMMC、U 盘等设备的解码播放和编码录音，最多支持同时三路解码；
* 支持 a/b/e、ump3、f1a/f1b/f1c/f1x、midi、标准 mp3、wav、opus 的七种解码播放；
* 支持标准 mp2、a、ump2、opus 的四种编码录音；
* 支持变速变调、echo、变音、啸叫抑制、PCM_EQ 等音效；
* 支持解码 MIO 功能；
* 支持传统玩具应用功能，包含解码 USB 从机、Linein、扩音、录音、等应用；
* AUDIO_DAC 支持单声道单端输出，支持 8K~96k 等 12 种采样率；
* AUDIO_APA（Class-D 直驱喇叭）支持单声道差分输出，支持 32K~48k 等 3 种采样率；
* AUDIO_ADC 支持单声道单端/差分输入，支持 8K~48k 等 9 种采样率；
* 支持硬件的重采样；
* 支持 SOFT OFF 关机和 POWER DOWN 休眠，关机功耗 2uA+，休眠功耗 61uA+,后续将会大幅度优化；

MCU信息
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

社区
--------------

* 技术交流群[钉钉](./doc/stuff/dingtalk.jpg)


免责声明
------------

AW30N_SDK 支持AW30N 系列芯片开发.
AW30N 系列芯片支持了通用MCU 常见应用，可以作为开发，评估，样品，甚至量产使用，对应SDK 版本见Release
