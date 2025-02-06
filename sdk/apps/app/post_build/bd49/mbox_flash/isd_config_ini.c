// *INDENT-OFF*
#include "app_config.h"
#define _CAT2(a,b) a ## _ ## b
#define CAT2(a,b) _CAT2(a,b)
#define _CAT3(a,b,c) a ## _ ## b ##  _  ## c
#define CAT3(a,b,c) _CAT3(a,b,c)
#define _CAT4(a,b,c,d) a ## _ ## b ##  _  ## c ## _ ## d
#define CAT4(a,b,c,d) _CAT4(a,b,c,d)
#define _CAT5(a,b,c,d,e) a ## _ ## b ##  _  ## c ## _ ## d ## _ ## e
#define CAT5(a,b,c,d,e) _CAT5(a,b,c,d,e)
//#####################################################
//#
//#   配置数据按照 长度+配置名字+数据的方式存储
//#
//#####################################################

[EXTRA_CFG_PARAM]
NEW_FLASH_FS=YES;
CHIP_NAME=AW30N;//8
ENTRY=0x4000100;//程序入口地址
PID=BD49;//长度16byte,示例：芯片封装_应用方向_方案名称
VID=0.01;
CHECK_OTA_BIN=YES;//检测ota.bin

RESERVED_OPT=0;//入口地址为0x1E00120需要定义该配置项

//DOWNLOAD_MODEL=SERIAL;
DOWNLOAD_MODEL=usb;
SERIAL_DEVICE_NAME=JlVirtualJtagSerial;
SERIAL_BARD_RATE=1000000;
SERIAL_CMD_OPT=10;
SERIAL_CMD_RATE=100; [n*10000]
SERIAL_CMD_RES=0;
SERIAL_INIT_BAUD_RATE=9600;
LOADER_BAUD_RATE=1000000;
LOADER_ASK_BAUD_RATE=1000000;
SERIAL_SEND_KEY=YES;
BEFORE_LOADER_WAIT_TIME=150;
INTERNAL_DIR_ALIGN=2;

//#NEED_RESERVED_4K=YES;//关闭4K保留
NEED_RESERVED_4K=YES;
NEED_RESERVED_AREA=YES;

//#是否实时根据FLASH支持的对齐方式组织文件（256字节或4K字节对齐）；
SPECIAL_OPT=0
//#强制4k对齐
FORCE_4K_ALIGN=YES


[CHIP_VERSION]
SUPPORTED_LIST=A

//#####################################################    UBOOT配置项，请勿随意调整顺序    ##################################################
[SYS_CFG_PARAM]
//#clk [0-255]
//#data_width[0 1 2 3 4] 3的时候uboot自动识别2或者4线
//#mode:
//#     0 RD_OUTPUT,       1 cmd       1 addr
//#     1 RD_I/O,          1 cmd       x addr
//#     2 RD_I/O_CONTINUE] no_send_cmd x add
//#SPI = data_width,clk,mode;
//##AUTH_CODE=1;
SPI=2_3_0_0;  #data_width,clk,mode;
//#OSC=btosc;
//#OSC_FREQ=24MHz; #[24MHz 12MHz]
//#SYS_CLK=48MHz; #[48MHz 24MHz]
UTTX=PA05;//uboot串口tx
UTBD=1000000;//uboot串口波特率
UTRX=PA00;           //串口升级, 默认PA00
//#RESET=PB01_08_0;   //port口_长按时间_有效电平（长按时间有00、04、08三个值可选，单位为秒，当长按时间为00时，则关闭长按复位功能。）
//#CACHE_WAY=1;// 范围1~4
//#WAIT_TIME=10;// * 100ms
#if TFG_EXT_FLASH_EN
EX_FLASH=CAT2(TFG_SPI_CS_PORT,1A_NULL);  //#cs,spi1,A,flashpower
#if TFG_SPI_UNIDIR_MODE_EN
//单线1bit
EX_FLASH_IO=CAT5(0,TFG_SPI_CLK_PORT,TFG_SPI_DO_PORT,TFG_SPI_DI_PORT,NULL_NULL);  //#data_width,clk,do,di,d2,d3
#else
//双线1bit
EX_FLASH_IO=CAT5(2,TFG_SPI_CLK_PORT,TFG_SPI_DO_PORT,TFG_SPI_DI_PORT,NULL_NULL);  //#data_width,clk,do,di,d2,d3
#endif
#endif

//########sd卡升级时需要配置对应的IO口###############################################
LATCH_IO = PA01&0_PA02&0_PA03&0;    //格式为 PA01&0_PA02&0_PA03&0, &0/1表示输出低或输出高
//#############################################################################################################################################

[FW_ADDITIONAL]
FILE_LIST=(file=ota.bin:type=100)

//####################################################
[TOOL_CONFIG]
1TO2_MIN_VER=2.27.8
1TO8_MIN_VER=3.1.22
BTBOX_MIN_VER=1.3.1
//####################################################

//####################################################
//#FLASH_WRITE_PROTECT=NO;

//#[BURNER_PASSTHROUGH_CFG]
//##AUTH_CODE_IN_FLASH=1;

[BURNER_OPTIONS]
LVD=2.5v
//########flash空间使用配置区域###############################################
//#PDCTNAME:    产品名，对应此代码，用于标识产品，升级时可以选择匹配产品名
//#BOOT_FIRST:  1=代码更新后，提示APP是第一次启动；0=代码更新后，不提示
//#UPVR_CTL：   0：不允许高版本升级低版本   1：允许高版本升级低版本
//#XXXX_ADR:    区域起始地址  AUTO：由工具自动分配起始地址
//#XXXX_LEN:    区域长度      CODE_LEN：代码长度
//#XXXX_OPT:    区域操作属性
//#
//#
//#
//#操作符说明  OPT:
//#   0:  下载代码时擦除指定区域
//#   1:  下载代码时不操作指定区域
//#   2:  下载代码时给指定区域加上保护
//############################################################################
[RESERVED_CONFIG]

BTIF_ADR=AUTO;
BTIF_LEN=0x1000;
BTIF_OPT=1;

PRCT_ADR=0;
PRCT_LEN=CODE_LEN;
PRCT_OPT=2;

VM_ADR=0;
VM_LEN=24K;
VM_OPT=1;

EEPROM_ADR=AUTO;
EEPROM_LEN=24K;
EEPROM_OPT=1;

EXIF_ADR=AUTO;
EXIF_LEN=4K;
EXIF_OPT=1;

[BURNER_CONFIG]
SIZE=32;

