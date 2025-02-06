










set ELF_NAME=%1%
cd /d %~dp0

set OBJDUMP=C:\JL\pi32\bin\llvm-objdump.exe
set OBJCOPY=C:\JL\pi32\bin\llvm-objcopy.exe

set OBJSIZEDUMP=C:\JL\pi32\bin\llvm-objsizedump.exe
%OBJSIZEDUMP% -lite -skip-zero -enable-dbg-info %ELF_NAME%.elf > %ELF_NAME%.txt

%OBJDUMP% -d -print-imm-hex -print-dbg %ELF_NAME%.elf > %ELF_NAME%.lst
%OBJCOPY% -O binary -j .app_code %ELF_NAME%.elf %ELF_NAME%.bin
%OBJCOPY% -O binary -j .data %ELF_NAME%.elf data.bin



%OBJDUMP% -section-headers %ELF_NAME%.elf




copy /b %ELF_NAME%.bin+data.bin app.bin



@echo *******************************************************************************************************
@echo BD49 mbox_flash
@echo *******************************************************************************************************
@echo % date %
cd / d % ~dp0

::isd_download.exe -tonorflash -dev bd49 -boot 0x3f01400 -div8 -wait 300 -uboot uboot.boot -app app.bin 0x20000 -res dir_song dir_eng dir_poetry dir_story

isd_download.exe -tonorflash -dev bd49 -boot 0x3f01400 -div8 -wait 300 -uboot uboot.boot -app app.bin 0x80000 -res dir_a dir_song dir_eng dir_poetry dir_story dir_midi midi_cfg dir_notice dir_bin_f1x cfg_tool.bin
::-format all
@REM 常用命令说明
@rem - format vm
@rem - format all
@rem - reboot 500

ufw_maker.exe -fw_to_ufw jl_isd.fw
copy jl_isd.ufw update.ufw

ping / n 2 127.1 > null
IF EXIST null del null
