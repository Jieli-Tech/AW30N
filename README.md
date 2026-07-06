[tag download]:https://github.com/Jieli-Tech/AW30N/tags
[tag_badgen]:https://img.shields.io/github/v/tag/Jieli-Tech/AW30N?style=plastic&logo=bluetooth&label=tag&labelColor=ffffff&color=informational

# fw-AW30N_BLE_SDK  [![tag][tag_badgen]][tag download]

<div align="center">

**杰理 AW30N 系列 BLE 通用 MCU SDK 固件程序**

[English](./README-en.md) · [文档中心](https://doc.zh-jieli.com/AW30/zh-cn/master/index.html) · [SDK 版本历史](doc/AW30N_SDK_发布版本信息.pdf) · [报告问题](https://github.com/Jieli-Tech/AW30N/issues)

</div>

---

## 目录

- [一、概述](#一概述)
- [二、支持的芯片与平台](#二支持的芯片与平台)
- [三、环境搭建](#三环境搭建)
- [四、快速开始](#四快速开始)
- [五、工程结构](#五工程结构)
- [六、应用与示例](#六应用与示例)
- [七、编译指南](#七编译指南)
- [八、烧录与升级](#八烧录与升级)
- [九、配置说明](#九配置说明)
- [十、常见问题](#十常见问题)
- [十一、社区与支持](#十一社区与支持)
- [十二、免责声明](#十二免责声明)

---

## 一、概述

`fw-AW30N_BLE_SDK` 是杰理科技为 AW30N 系列芯片提供的 BLE 通用 MCU SDK 开发包。本系列芯片为带 BLE 5.4 蓝牙功能的 32bit DSP MCU，主要面向以下应用场景：

| 应用类型 | 典型产品 |
|---------|---------|
| **BLE 蓝牙** | 蓝牙遥控器、蓝牙对讲机、BLE Dongle |
| **语音玩具** | 故事机、学习机、发声玩具 |
| **小音箱** | 音乐播放器、扩音器 |
| **通用 MCU** | 智能控制、传感器采集、USB 音频设备 |

### 核心特性

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


本仓库包含 SDK Release 版本代码及示例工程，需配合对应命名规则的库文件 (`lib.a`) 进行编译。

---

## 二、支持的芯片与平台

### 2.1 SoC 系列

AW30N 全系主推BLE蓝牙语音遥控器、BLE对讲机、小音箱、语音玩具。

> 芯片型号/规格书/原理图资料请查阅：[doc/ 目录](doc/)

### 2.2 蓝牙协议支持

| 蓝牙规范 | QDID | 状态 |
|---------|-----|------|
| **Core v5.4** | 223418 | ✅ |


---

## 三、环境搭建

### 3.1 前提条件

| 系统 | 说明 |
|------|------|
| **Windows** | 推荐使用 Code::Blocks IDE 编译 |
| **Linux** | Makefile 命令行编译（需要重写 download_sh.c 脚本适配 Linux 环境） |
| **macOS** | 需自行配置交叉编译工具链 |

### 3.2 安装编译工具链

1. 下载并安装 **杰理编译工具链**：[下载链接](https://doc.zh-jieli.com/Tools/zh-cn/dev_tools/dev_env/index.html)
2. Linux 用户可从此处下载：[pkgman.jieliapp.com](http://pkgman.jieliapp.com/doc/all)
   - 下载后解压到 `/opt/jieli` 目录
   - 确保 `/opt/jieli/pi32/bin/clang` 存在
3. 安装完成后验证：

```bash
# 验证工具链是否安装成功
clang --version
```

### 3.3 安装烧录工具

| 工具 | 用途 | 获取方式 |
|------|------|---------|
| **USB 升级工具** | 将固件烧录到目标板 | [申请链接](https://item.taobao.com/item.htm?id=620295020803) · [使用文档](https://doc.zh-jieli.com/Tools/zh-cn/dev_tools/forced_upgrade/index.html) |
| **生产烧写工具** | 量产/裸片烧写 | **代理商处** · [使用文档](https://doc.zh-jieli.com/Tools/zh-cn/mass_prod_tools/burner_1tuo2/index.html) |
| **无线测试盒** | 空中升级/射频标定/产品测试 | [申请链接](https://item.taobao.com/item.htm?id=620942507511) · [使用文档](https://doc.zh-jieli.com/Tools/zh-cn/mass_prod_tools/testbox_1tuo2/index.html) |

---

### 3.5 音频工具

打包、音频文件转换、MIDI 等通用音频工具：[下载链接](https://pan.baidu.com/s/1ajzBF4BFeiRFpDF558ER9w#list/path=%2F) 提取码：`3jey`

---

## 四、快速开始

### 4.1 克隆仓库

```bash
git clone https://github.com/Jieli-Tech/AW30N.git
cd AW30N/sdk
```

### 4.2 工程入口

SDK 包含以下应用工程，位于 `sdk/` 根目录：

| 工程文件 | 芯片 | 应用类型 |
|---------|------|---------|
| `AW30N_mbox_flash.cbp` | AW30N 全系列 | BLE 蓝牙 / 小音箱 / 音频播放 |

### 4.3 应用代码入口

```
sdk/apps/app/src/mbox_flash/    # 蓝牙语音遥控器/对讲机/小音箱/音频播放应用
```

### 4.4 编译并烧录

**方式一：Code::Blocks（推荐 Windows 用户）**

1. 双击打开 `AW30N_mbox_flash.cbp` 工程文件
2. 点击 **Build → Build**（Ctrl+F9）
3. 编译成功后，使用 USB 升级工具烧录生成的固件

**方式二：Makefile 命令行**

```bash
# Windows 用户
双击 sdk/make_prompt.bat 打开命令行环境

# 编译
make -j4

# 显示编译详情
make VERBOSE=1 -j4
```

> **💡 提示**：编译前请确保 USB 升级工具正确连接且目标板已进入编程模式。

**方式三：VS Code 编译**

仓库已预配置 VS Code 任务，按 `Ctrl+Shift+B` 即可选择编译目标。

---

## 五、工程结构

```
fw-AW30N/
├── sdk/                           # SDK 主目录
│   ├── apps/                      # 应用层代码
│   │   ├── app/                   #   应用入口源码
│   │   │   ├── src/               #     应用源码
│   │   │   │   └── mbox_flash/    #       BLE 蓝牙/小音箱/音频播放应用
│   │   │   ├── bsp/               #     板级支持包（BSP）
│   │   │   └── post_build/        #     编译后处理脚本与工具
│   │   └── include_lib/           #   头文件与预编译库
│   │       ├── cpu/               #     CPU 平台头文件
│   │       ├── decoder/           #     解码器 API 头文件
│   │       ├── encoder/           #     编码器 API 头文件
│   │       ├── audio/             #     音频 API 头文件
│   │       ├── device/            #     设备驱动头文件
│   │       ├── common/            #     公共头文件
│   │       ├── config/            #     配置头文件
│   │       ├── msg/               #     消息机制
│   │       ├── update/            #     固件升级
│   │       └── liba/              #     预编译库 (.a)
│   ├── tools/                     # 编译工具与脚本
│   │   ├── make_prompt.bat        #   Windows 编译命令行入口
│   │   └── utils/                 #   工具集（make、rm 等）
│   ├── Makefile                   # 顶层 Makefile
│   └── *.cbp                      # Code::Blocks 工程文件
├── doc/                           # 文档
│   ├── datasheet/                 #   芯片规格书
│   ├── schematic/                 #   原理图
│   ├── stuff/                     #   杂项（钉钉群等）
│   ├── AW30N_SDK手册_V1.7.pdf     #   SDK 手册
│   ├── AW30N_SDK_发布版本信息.pdf  #   SDK 发布版本信息
│   ├── AW30N_芯片手册_V1.1.pdf     #   芯片手册
│   ├── AW30N硬件设计指南V1.2.pdf    #   硬件设计指南
│   └── 杰理科技AW30N系列芯片选型表_20240816.pdf  # 芯片选型表
└── README.md                      # 本文件
```

---

## 六、应用与示例

### 6.1 BLE 蓝牙/音频播放应用 (`apps/app/src/mbox_flash/`)

| 功能 | 说明 |
|-------|------|
| **BLE 蓝牙** | BLE 5.4 从机/主机，GATT 数据收发，蓝牙 OTA 升级 |
| **BLE 遥控器** | BLE 从机遥控器，低功耗广播与连接 |
| **BLE 对讲机** | 2.4GHz 私有协议对讲机方案 |
| **BLE Dongle** | USB BLE Dongle 主机 |
| **音乐播放** | FLASH、SD 卡、U 盘文件播放（支持 MP3/WAV/OPUS 等） |
| **MIDI 演奏** | MIDI 合成与播放 |
| **录音** | MP2/A/UMP2/OPUS 格式编码录音 |
| **USB Device** | USB 从设备（Speaker / MIC / HID / MSD） |
| **LINEIN** | 线路输入 |
| **扩音** | 扩音/喊话功能 |

适用领域：蓝牙遥控器、蓝牙对讲机、BLE Dongle、小音箱、蓝牙玩具、AI 语音交互等。

---

## 七、编译指南

### 7.1 编译命令速查表

以下命令在 `sdk/` 目录下执行：

| 目标 | 命令 |
|------|------|
| **编译** | `make -j4` |
| **编译（verbose）** | `make VERBOSE=1 -j4` |
| **清理** | `make clean` |

### 7.2 Code::Blocks 编译（推荐 Windows 用户）

1. 确保已安装杰理编译工具链
2. 双击 `AW30N_mbox_flash.cbp` 工程文件打开 Code::Blocks
3. 点击 **Build → Build**（Ctrl+F9）
4. 编译成功后在 `post_build/` 目录下生成固件

### 7.3 Makefile 编译

```bash
# Windows 用户
双击 sdk/make_prompt.bat 打开命令行环境
make -j4

# Linux 用户（需要自行修改download_sh.c文件适配Linux）
cd sdk
make -j`nproc`
```

### 7.4 常见编译错误

| 错误提示 | 解决方法 |
|---------|---------|
| `clang: command not found` | 未安装杰理编译工具链，或环境变量未配置 |
| `cannot find -lxxx` | 缺少对应的 `.a` 库文件，检查 `apps/include_lib/liba/` 目录 |
| `make: command not found` | Windows 下使用 `tools/make_prompt.bat` 打开编译命令环境 |
| 链接错误 | 检查 Makefile target 是否匹配当前芯片型号 |

---

## 八、烧录与升级

### 8.1 首次烧录

1. **连接硬件**：将开发板通过 **USB** 或者 **USB 升级工具** 连接到 PC
2. **进入编程模式**：
    - 方式一（USB）：按住开发板上的烧录按键，然后复位或重新上电
    - 方式二（USB/UART）：通过 USB 升级工具进入编程模式
3. **打开 USB 升级工具**：启动烧录上位机
4. **选择固件**：选择编译生成的固件文件
5. **开始烧录**：点击下载按钮，等待烧录完成

> **注意**：烧录前请确保 USB 升级工具正确连接且目标板已进入编程模式。关于 ISD_CONFIG.INI 配置详见 [ISD 配置说明](https://doc.zh-jieli.com/Tools/zh-cn/dev_tools/toolchains/ini_cfg.html)。

### 8.2 生产烧写

量产场景请使用杰理生产烧写工具（一拖二 / 一拖八），支持裸片烧写。详见 [一拖二烧写器使用说明](https://doc.zh-jieli.com/Tools/zh-cn/mass_prod_tools/burner_1tuo2/index.html) · [一拖八烧写器使用说明](https://doc.zh-jieli.com/Tools/zh-cn/mass_prod_tools/burner_1tuo8/index.html)

### 8.3 OTA 升级

支持多种升级方式：
- 蓝牙 OTA 升级（手机蓝牙升级、测试盒蓝牙升级）
- USB 升级（手机 USB 升级）
- U 盘 / SD 卡升级
- 测试盒串口升级

---

## 九、配置说明

- 编辑 `sdk/apps/app/src/mbox_flash/app_config.h` 可配置目标应用的功能开关
- 可通过 BLE Profile 制作工具配置 GATT 服务

---

## 十、常见问题

### 10.1 开发流程相关

**Q: 如何创建一个新的工程？**
A: 基于现有的 `.cbp` 工程和 `apps/app/src/` 中的应用代码进行修改，配置对应用例即可。

**Q: 如何切换不同的芯片型号？**
A: 在配置中选择对应的芯片型号，SDK 已为全系列预配置了统一的编译入口。

### 10.2 编译相关

**Q: Windows 下编译报错 `make` 不是有效命令？**
A: 使用 `sdk/make_prompt.bat` 进入预配置的命令行环境，该脚本已设置好所有环境变量和 `make` 的路径。

**Q: 如何加快编译速度？**
A: 使用 `-j` 参数进行并行编译，如 `make -j4`（数字为并行任务数）。

### 10.3 调试技巧

- **串口日志**：可通过 UART 输出调试日志
- **BLE 抓包**：可使用 BLE Dongle 进行空中抓包分析

---

## 十一、社区与支持

### 技术交流

| 平台 | 群号/链接 | 状态 |
|------|-----------|------|
| **钉钉技术交流群** | 见 [群二维码](doc/stuff/ding_talk.jpg) | ✅ 可加入 |

### 资源链接

| 资源 | 链接 |
|------|------|
| 📖 **在线文档中心** | [doc.zh-jieli.com/AW30](https://doc.zh-jieli.com/AW30/zh-cn/master/index.html) |
| 📚 **SDK 版本历史** | [SDK 发布版本信息](doc/AW30N_SDK_发布版本信息.pdf) |
| 🔧 **SDK 快速入门** | [SDK 手册](doc/AW30N_SDK手册_V1.7.pdf) |
| 📖 **芯片手册** | [AW30N 芯片手册](doc/AW30N_芯片手册_V1.1.pdf) |
| 📐 **硬件设计指南** | [硬件设计指南](doc/AW30N硬件设计指南V1.2.pdf) |
| 📄 **芯片选型** | [选型表](doc/杰理科技AW30N系列芯片选型表_20240816.pdf) |
| 🎬 **视频教程** | [Bilibili 主页](https://space.bilibili.com/3493277347088769/dynamic) |
| 🎵 **MIDI 开发手册** | [MIDI 应用开发文档](https://doc.zh-jieli.com/MIDI/zh-cn/master/index.html) |
| 📦 **FAE 支持** | [FAE 支持仓库](https://gitee.com/jieli-tech_fae/fw-jl) |
| 🛒 **开发板/烧录工具购买** | [杰理官方店铺](https://shop321455197.taobao.com/) |
| 🐛 **问题反馈** | [Github Issues](https://github.com/Jieli-Tech/AW30N/issues) |

---

## 十二、免责声明

`fw-AW30N_BLE_SDK` 支持 AW30N 系列芯片开发。本系列芯片支持通用 MCU 常见应用，可作为开发、评估、样品及量产使用，对应 SDK 版本请见 [SDK 版本历史](doc/AW30N_SDK_发布版本信息.pdf)。

---

<div align="center">
  <sub>Copyright &copy; 珠海杰理科技股份有限公司. All rights reserved.</sub>
</div>
