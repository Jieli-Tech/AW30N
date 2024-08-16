// *INDENT-OFF*
#include "app_config.h"

set ELF_NAME=%1%
cd /d %~dp0

set OBJDUMP=C:\JL\pi32\bin\llvm-objdump.exe
set OBJCOPY=C:\JL\pi32\bin\llvm-objcopy.exe

%OBJDUMP% -d -print-imm-hex -print-dbg %ELF_NAME%.elf > %ELF_NAME%.lst
%OBJCOPY% -O binary -j .app_code %ELF_NAME%.elf  %ELF_NAME%.bin
%OBJCOPY% -O binary -j .data %ELF_NAME%.elf  data.bin
%OBJDUMP% -section-headers  %ELF_NAME%.elf
copy /b %ELF_NAME%.bin+data.bin app.bin


@echo *******************************************************************************************************
@echo 			                BD49 mbox_flash
@echo *******************************************************************************************************
@echo % date %
cd / d % ~dp0

::isd_download.exe -tonorflash -dev bd49 -boot 0x3f01400 -div8 -wait 300  -uboot uboot.boot -app app.bin 0x20000 -res dir_song dir_eng dir_poetry dir_story

isd_download.exe -tonorflash -dev bd49 -boot 0x3f01400 -div8 -wait 300  -uboot uboot.boot -app app.bin 
::0x6f000 -res dir_a dir_song dir_eng dir_poetry dir_story dir_midi midi_cfg dir_notice dir_bin_f1x
::-format all
@REM 常用命令说明
@rem - format vm        // 擦除VM 区域
@rem - format all       // 擦除所有
@rem - reboot 500       // reset chip, valid in JTAG debug

ufw_maker.exe -fw_to_ufw jl_isd.fw
copy jl_isd.ufw update.ufw

ping / n 2 127.1 > null
IF EXIST null del null
