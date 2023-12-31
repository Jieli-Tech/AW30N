#ifndef __UI_API_H__
#define __UI_API_H__
#include "typedef.h"
#include "app_config.h"
#include "led5x7_driver.h"

enum {
    MENU_POWER_UP = 0,
    MENU_IDLE,
    MENU_WAIT,
#ifdef USB_DEVICE_EN
    MENU_PC_MAIN,
    MENU_PC_VOL_UP,
    MENU_PC_VOL_DOWN,
#endif
    MENU_AUX_MAIN,

    MENU_PLAY,
    MENU_PLAYMODE,
    MENU_MAIN_VOL,
    MENU_HW_EQ,
    MENU_DEC_EQ,
    MENU_NOFILE,
    MENU_NODEVICE,
    MENU_PLAY_TIME,
    MENU_FILENUM,
    MENU_INPUT_NUMBER,
    MENU_MUSIC_MAIN,
    MENU_PAUSE,
    MENU_FM_MAIN,
    MENU_FM_DISP_FRE,
    MENU_FM_FIND_STATION,
    MENU_FM_CHANNEL,
    MENU_USBREMOVE,
    MENU_SDREMOVE,
    MENU_SCAN_DISK,

    MENU_200MS_REFRESH = 0x80,
    MENU_100MS_REFRESH,
    MENU_SET_EQ,
    MENU_SET_PLAY_MODE,
    MENU_HALF_SEC_REFRESH,
    MENU_POWER_DOWN,
    MENU_MAIN = 0xFF,
};

typedef struct _UI_VAR {
    u8  bCurMenu;
    u8  bMainMenu;
    u8  bMenuReturnCnt;
    int  bCurArg;
} UI_VAR;

#define UI_RETURN				3//n * 500ms

void UI_menu_api(u8 menu, int arg);
void UI_init_api(void);

#if UI_ENABLE
extern UI_VAR UI_var;       /*UI 显示变量*/
#define UI_init()	        UI_init_api()
#define UI_menu(x, y)		UI_menu_api(x, y)
#define SET_UI_MAIN(x)	    UI_var.bMainMenu = x
#else
#define UI_init(...)
#define UI_menu(...)
#define SET_UI_MAIN(...)
#endif

#endif
