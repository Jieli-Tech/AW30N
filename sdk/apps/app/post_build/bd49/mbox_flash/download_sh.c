// *INDENT-OFF*
#include "app_config.h"
if [ -f $1.bin ];
then rm $1.bin;
fi
if [ -f $1_data.bin ];
then rm $1_data.bin;
fi
if [ -f $1_data1.bin ];
then rm $1_data1.bin;
fi
if [ -f $1.lst ];
then rm $1.lst;
fi

${OBJDUMP} -d -print-imm-hex -print-dbg $1.elf > $1.lst
${OBJSIZEDUMP} -lite -skip-zero -enable-dbg-info ${1}.elf | sort -k 1 >  ${1}.txt
${OBJCOPY} -O binary -j .app_code $1.elf  $1.bin
${OBJCOPY} -O binary -j .data $1.elf  data.bin
#if ICACHE_RAM_TO_RAM_ENABLE
${OBJCOPY} -O binary -j .cache_ram $1.elf  cache_ram.bin
#endif
${OBJDUMP} -section-headers  $1.elf

#if ICACHE_RAM_TO_RAM_ENABLE
cat $1.bin data.bin cache_ram.bin > app.bin
#else
cat $1.bin data.bin > app.bin
#endif

//host-client -project ${NICKNAME} -mode flash_debug -f app.bin $1.elf isd_config.ini uboot.boot bd49loader.bin
host-client -project ${NICKNAME} -mode flash_debug -f app.bin $1.elf uboot.boot uboot.boot_debug bd49loader.bin  bd49loader.uart ota*
