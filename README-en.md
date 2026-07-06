[tag download]:https://github.com/Jieli-Tech/AW30N/tags
[tag_badgen]:https://img.shields.io/github/v/tag/Jieli-Tech/AW30N?style=plastic&logo=bluetooth&label=tag&labelColor=ffffff&color=informational

# fw-AW30N_BLE_SDK  [![tag][tag_badgen]][tag download]

<div align="center">

**Jieli AW30N Series BLE Universal MCU SDK Firmware**

[中文](./README.md) · [Documentation Center](https://doc.zh-jieli.com/AW30/zh-cn/master/index.html) · [SDK Release History](doc/AW30N_SDK_发布版本信息.pdf) · [Report an Issue](https://github.com/Jieli-Tech/AW30N/issues)

</div>

---

## Table of Contents

- [1. Overview](#1-overview)
- [2. Supported Chips and Platforms](#2-supported-chips-and-platforms)
- [3. Environment Setup](#3-environment-setup)
- [4. Quick Start](#4-quick-start)
- [5. Project Structure](#5-project-structure)
- [6. Applications and Examples](#6-applications-and-examples)
- [7. Build Guide](#7-build-guide)
- [8. Flashing and Upgrade](#8-flashing-and-upgrade)
- [9. Configuration](#9-configuration)
- [10. FAQ](#10-faq)
- [11. Community and Support](#11-community-and-support)
- [12. Disclaimer](#12-disclaimer)

---

## 1. Overview

`fw-AW30N_BLE_SDK` is a BLE universal MCU SDK development package provided by Jieli Technology for the AW30N series chips. These chips are 32-bit DSP MCUs with integrated BLE 5.4 Bluetooth, primarily targeted at the following application scenarios:

| Application Type | Typical Products |
|---------|---------|
| **BLE Bluetooth** | Bluetooth remote controls, Bluetooth intercoms, BLE Dongles |
| **Voice Toys** | Storytelling machines, learning devices, sound-emitting toys |
| **Mini Speakers** | Music players, loudspeakers |
| **General MCU** | Smart control, sensor acquisition, USB audio devices |

### Core Features

* Supports single-mode BLE 5.4 Bluetooth
* Supports full GATT service and simple GATT service
* Full GATT service is based on standard GATT protocol with complete GATT profile
* Simple GATT service is tailored from standard GATT protocol, supporting simple data transmission
* New BLE slave remote control, BLE Dongle host, and BLE intercom application examples
* BLE slave remote control broadcast power consumption: 290 µA+ (disconnected), standby power consumption: 130 µA+ (connected)
* Supports USB flash drive / SD card upgrade
* Supports test box UART upgrade
* Supports test box Bluetooth upgrade
* Supports phone Bluetooth OTA upgrade
* Supports phone USB upgrade
* Supports decoding playback and encoding recording from system FLASH, resource FLASH, SDMMC, USB flash drives, with up to three simultaneous decode streams
* Supports seven decoding formats: a/b/e, ump3, f1a/f1b/f1c/f1x, midi, standard mp3, wav, opus
* Supports four encoding formats for recording: standard mp2, a, ump2, opus
* Supports variable speed/pitch, echo, voice changer, howling suppression, PCM_EQ and other audio effects
* Supports MIO decoding function
* Supports traditional toy application functions including USB slave decoding, Linein, loudspeaker, recording, etc.
* AUDIO_DAC supports mono single-ended output, 12 sample rates from 8K–96k
* AUDIO_APA (Class-D direct speaker drive) supports mono differential output, 3 sample rates from 32K–48k
* AUDIO_ADC supports mono single-ended/differential input, 9 sample rates from 8K–48k
* Supports hardware resampling
* Supports SOFT OFF shutdown and POWER DOWN sleep, shutdown power consumption 2 µA+, sleep power consumption 61 µA+ (to be greatly optimized in future)

This repository contains SDK release versions and sample projects. Compilation requires the corresponding library files (`lib.a`) that follow the naming convention.

---

## 2. Supported Chips and Platforms

### 2.1 SoC Families

The AW30N series focuses on BLE Bluetooth voice remote controls, BLE intercoms, mini speakers, and voice toys.

> For chip models, datasheets, and schematic resources, see: [doc/ directory](doc/)

### 2.2 Bluetooth Protocol Support

| Bluetooth Spec | QDID | Status |
|---------|-----|------|
| **Core v5.4** | 223418 | ✅ |


---

## 3. Environment Setup

### 3.1 Prerequisites

| System | Description |
|------|------|
| **Windows** | Code::Blocks IDE recommended for compilation |
| **Linux** | Command-line compilation via Makefile (requires rewriting `download_sh.c` for Linux compatibility) |
| **macOS** | Cross-compilation toolchain must be configured manually |

### 3.2 Install the Build Toolchain

1. Download and install the **Jieli Build Toolchain**: [Download Link](https://doc.zh-jieli.com/Tools/zh-cn/dev_tools/dev_env/index.html)
2. Linux users can download from: [pkgman.jieliapp.com](http://pkgman.jieliapp.com/doc/all)
   - Extract to `/opt/jieli` after downloading
   - Ensure `/opt/jieli/pi32/bin/clang` exists
3. Verify installation:

```bash
# Verify toolchain installation
clang --version
```

### 3.3 Install Flashing Tools

| Tool | Purpose | How to Obtain |
|------|------|---------|
| **USB Upgrade Tool** | Flash firmware to the target board | [Application Link](https://item.taobao.com/item.htm?id=620295020803) · [User Guide](https://doc.zh-jieli.com/Tools/zh-cn/dev_tools/forced_upgrade/index.html) |
| **Mass Production Burner** | Mass production / bare-die programming | **From distributor** · [User Guide](https://doc.zh-jieli.com/Tools/zh-cn/mass_prod_tools/burner_1tuo2/index.html) |
| **Wireless Test Box** | OTA upgrade / RF calibration / product testing | [Application Link](https://item.taobao.com/item.htm?id=620942507511) · [User Guide](https://doc.zh-jieli.com/Tools/zh-cn/mass_prod_tools/testbox_1tuo2/index.html) |

---

### 3.4 Audio Tools

Universal audio tools for packaging, audio file conversion, MIDI, etc.: [Download Link](https://pan.baidu.com/s/1ajzBF4BFeiRFpDF558ER9w#list/path=%2F) Password: `3jey`

---

## 4. Quick Start

### 4.1 Clone the Repository

```bash
git clone https://github.com/Jieli-Tech/AW30N.git
cd AW30N/sdk
```

### 4.2 Project Entry Points

The SDK includes the following application projects, located in the `sdk/` root directory:

| Project File | Chip | Application Type |
|---------|------|---------|
| `AW30N_mbox_flash.cbp` | AW30N All Series | BLE Bluetooth / Mini Speakers / Audio player |

### 4.3 Application Code Entry Points

```
sdk/apps/app/src/mbox_flash/    # BLE remote control / Intercom / Mini speaker / Audio player application
```

### 4.4 Build and Flash

**Method 1: Code::Blocks (Recommended for Windows users)**

1. Double-click the `AW30N_mbox_flash.cbp` project file
2. Click **Build → Build** (Ctrl+F9)
3. After a successful build, use the USB Upgrade Tool to flash the generated firmware

**Method 2: Makefile (Command Line)**

```bash
# Windows users
Double-click sdk/make_prompt.bat to open the command-line environment

# Build
make -j4

# Build with verbose output
make VERBOSE=1 -j4
```

> **💡 Tip**: Before building, ensure the USB Upgrade Tool is properly connected and the target board has entered programming mode.

**Method 3: VS Code Build**

The repository comes pre-configured with VS Code tasks. Press `Ctrl+Shift+B` to select a build target.

---

## 5. Project Structure

```
fw-AW30N/
├── sdk/                           # SDK root directory
│   ├── apps/                      # Application layer code
│   │   ├── app/                   #   Application entry source code
│   │   │   ├── src/               #     Application source
│   │   │   │   └── mbox_flash/    #       BLE Bluetooth / Mini speaker / Audio player app
│   │   │   ├── bsp/               #     Board Support Package (BSP)
│   │   │   └── post_build/        #     Post-build scripts and tools
│   │   └── include_lib/           #   Headers and precompiled libraries
│   │       ├── cpu/               #     CPU platform headers
│   │       ├── decoder/           #     Decoder API headers
│   │       ├── encoder/           #     Encoder API headers
│   │       ├── audio/             #     Audio API headers
│   │       ├── device/            #     Device driver headers
│   │       ├── common/            #     Common headers
│   │       ├── config/            #     Configuration headers
│   │       ├── msg/               #     Message mechanism
│   │       ├── update/            #     Firmware upgrade
│   │       └── liba/              #     Precompiled libraries (.a)
│   ├── tools/                     # Build tools and scripts
│   │   ├── make_prompt.bat        #   Windows build command-line launcher
│   │   └── utils/                 #   Utilities (make, rm, etc.)
│   ├── Makefile                   # Top-level Makefile
│   └── *.cbp                      # Code::Blocks project files
├── doc/                           # Documentation
│   ├── datasheet/                 #   Chip datasheets
│   ├── schematic/                 #   Schematics
│   ├── stuff/                     #   Miscellaneous (DingTalk group, etc.)
│   ├── AW30N_SDK手册_V1.7.pdf     #   SDK manual
│   ├── AW30N_SDK_发布版本信息.pdf  #   SDK release notes
│   ├── AW30N_芯片手册_V1.1.pdf     #   Chip manual
│   ├── AW30N硬件设计指南V1.2.pdf    #   Hardware design guide
│   └── 杰理科技AW30N系列芯片选型表_20240816.pdf  # Chip selection table
└── README.md                      # This file
```

---

## 6. Applications and Examples

### 6.1 BLE Bluetooth / Audio Player Application (`apps/app/src/mbox_flash/`)

| Feature | Description |
|-------|------|
| **BLE Bluetooth** | BLE 5.4 slave/host, GATT data transfer, Bluetooth OTA upgrade |
| **BLE Remote Control** | BLE slave remote control, low-power advertising and connection |
| **BLE Intercom** | 2.4 GHz proprietary protocol intercom solution |
| **BLE Dongle** | USB BLE Dongle host |
| **Music Playback** | FLASH, SD card, USB flash drive file playback (MP3/WAV/OPUS, etc.) |
| **MIDI Performance** | MIDI synthesis and playback |
| **Recording** | MP2/A/UMP2/OPUS format encoding and recording |
| **USB Device** | USB slave device (Speaker / MIC / HID / MSD) |
| **LINEIN** | Line input |
| **Loudspeaker** | Loudspeaker / voice amplifier |

Target domains: Bluetooth remote controls, Bluetooth intercoms, BLE Dongles, mini speakers, Bluetooth toys, AI voice interaction, etc.

---

## 7. Build Guide

### 7.1 Build Command Quick Reference

Run the following commands from the `sdk/` directory:

| Target | Command |
|------|------|
| **Build** | `make -j4` |
| **Build (verbose)** | `make VERBOSE=1 -j4` |
| **Clean** | `make clean` |

### 7.2 Code::Blocks Build (Recommended for Windows users)

1. Ensure the Jieli build toolchain is installed
2. Double-click the `AW30N_mbox_flash.cbp` project file to open Code::Blocks
3. Click **Build → Build** (Ctrl+F9)
4. After a successful build, the firmware will be generated in the `post_build/` directory

### 7.3 Makefile Build

```bash
# Windows users
Double-click sdk/make_prompt.bat to open the command-line environment
make -j4

# Linux users (requires modifying download_sh.c for Linux compatibility)
cd sdk
make -j`nproc`
```

### 7.4 Common Build Errors

| Error Message | Solution |
|---------|---------|
| `clang: command not found` | Jieli build toolchain is not installed, or environment variables are not configured |
| `cannot find -lxxx` | Missing corresponding `.a` library file; check the `apps/include_lib/liba/` directory |
| `make: command not found` | On Windows, use `tools/make_prompt.bat` to open the build command environment |
| Link errors | Verify that the Makefile target matches the current chip model |

---

## 8. Flashing and Upgrade

### 8.1 First-Time Flashing

1. **Connect Hardware**: Connect the development board to the PC via **USB** or **USB Upgrade Tool**
2. **Enter Programming Mode**:
    - Method 1 (USB): Hold the flash button on the development board, then reset or power-cycle
    - Method 2 (USB/UART): Use the USB Upgrade Tool to enter programming mode
3. **Launch USB Upgrade Tool**: Start the flashing host software
4. **Select Firmware**: Choose the compiled firmware file
5. **Start Flashing**: Click the download button and wait for completion

> **Note**: Before flashing, ensure the USB Upgrade Tool is properly connected and the target board has entered programming mode. For details on ISD_CONFIG.INI, see [ISD Configuration Guide](https://doc.zh-jieli.com/Tools/zh-cn/dev_tools/toolchains/ini_cfg.html).

### 8.2 Mass Production Flashing

For mass production scenarios, use Jieli's mass production burner (one-to-two / one-to-eight), which supports bare-die programming. See [One-to-Two Burner User Guide](https://doc.zh-jieli.com/Tools/zh-cn/mass_prod_tools/burner_1tuo2/index.html) · [One-to-Eight Burner User Guide](https://doc.zh-jieli.com/Tools/zh-cn/mass_prod_tools/burner_1tuo8/index.html)

### 8.3 OTA Upgrade

Multiple upgrade methods supported:
- Phone Bluetooth OTA upgrade
- Phone USB upgrade
- USB flash drive / SD card upgrade
- Test box UART upgrade
- Test box Bluetooth upgrade

---

## 9. Configuration

- Edit `sdk/apps/app/src/mbox_flash/app_config.h` to configure feature toggles for the target application
- GATT services can be configured via the BLE Profile Tool

---

## 10. FAQ

### 10.1 Development Workflow

**Q: How do I create a new project?**
A: Start from an existing `.cbp` project and the application code in `apps/app/src/`, then configure the corresponding application examples.

**Q: How do I switch between different chip models?**
A: Select the corresponding chip model in the configuration. The SDK provides a unified build entry for all chips in the series.

### 10.2 Build Issues

**Q: On Windows, `make` is reported as an invalid command?**
A: Use `sdk/make_prompt.bat` to enter the pre-configured command-line environment, which sets up all environment variables and the `make` path.

**Q: How can I speed up compilation?**
A: Use the `-j` flag for parallel compilation, e.g. `make -j4` (the number specifies the parallel job count).

### 10.3 Debugging Tips

- **UART Logging**: Debug logs can be output via UART
- **BLE Sniffing**: Use a BLE Dongle for over-the-air packet capture and analysis

---

## 11. Community and Support

### Technical Discussion

| Platform | Group / Link | Status |
|------|-----------|------|
| **DingTalk Tech Group** | See [Group QR Code](doc/stuff/ding_talk.jpg) | ✅ Joinable |

### Resource Links

| Resource | Link |
|------|------|
| 📖 **Online Documentation Center** | [doc.zh-jieli.com/AW30](https://doc.zh-jieli.com/AW30/zh-cn/master/index.html) |
| 📚 **SDK Release History** | [SDK Release Notes](doc/AW30N_SDK_发布版本信息.pdf) |
| 🔧 **SDK Quick Start** | [SDK Manual](doc/AW30N_SDK手册_V1.7.pdf) |
| 📖 **Chip Manual** | [AW30N Chip Manual](doc/AW30N_芯片手册_V1.1.pdf) |
| 📐 **Hardware Design Guide** | [Hardware Design Guide](doc/AW30N硬件设计指南V1.2.pdf) |
| 📄 **Chip Selection Guide** | [Selection Table](doc/杰理科技AW30N系列芯片选型表_20240816.pdf) |
| 🎬 **Video Tutorials** | [Bilibili Homepage](https://space.bilibili.com/3493277347088769/dynamic) |
| 🎵 **MIDI Development Manual** | [MIDI Application Development Guide](https://doc.zh-jieli.com/MIDI/zh-cn/master/index.html) |
| 📦 **FAE Support** | [FAE Support Repository](https://gitee.com/jieli-tech_fae/fw-jl) |
| 🛒 **Dev Board / Flashing Tool Purchase** | [Jieli Official Store](https://shop321455197.taobao.com/) |
| 🐛 **Issue Tracker** | [Github Issues](https://github.com/Jieli-Tech/AW30N/issues) |

---

## 12. Disclaimer

`fw-AW30N_BLE_SDK` supports development for the AW30N series chips. These chips support common general-purpose MCU applications and may be used for development, evaluation, sampling, and mass production. For the corresponding SDK version, please refer to [SDK Release History](doc/AW30N_SDK_发布版本信息.pdf).

---

<div align="center">
  <sub>Copyright &copy; Zhuhai Jieli Technology Co., Ltd. All rights reserved.</sub>
</div>
