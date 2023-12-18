#include "typedef.h"
#include "key.h"
#include "msg.h"
#include "app_config.h"
#include "rc_app.h"

#if KEY_AD_EN
#define ADKEY_REMOTE_CONTROLLER_SHORT_UP \
							/*00*/		MSG_CONFIRM,\
							/*01*/		MSG_UP,\
							/*02*/		MSG_DOWN,\
							/*03*/		MSG_VOL_UP,\
							/*04*/		MSG_VOL_DOWN,\
							/*05*/		MSG_SPEAKER,\
							/*06*/		NO_MSG,\
							/*07*/		NO_MSG,\
							/*08*/		NO_MSG,\
							/*09*/		NO_MSG,\

#define ADKEY_REMOTE_CONTROLLER_LONG \
							/*00*/		MSG_MUTE,\
							/*01*/		NO_MSG,\
							/*02*/		NO_MSG,\
							/*03*/		MSG_VOL_UP,\
							/*04*/		MSG_VOL_DOWN,\
							/*05*/		NO_MSG,\
							/*06*/		NO_MSG,\
							/*07*/		NO_MSG,\
							/*08*/		NO_MSG,\
							/*09*/		NO_MSG,\

#define ADKEY_REMOTE_CONTROLLER_HOLD \
							/*00*/		NO_MSG,\
							/*01*/		NO_MSG,\
							/*02*/		NO_MSG,\
							/*03*/		MSG_VOL_UP,\
							/*04*/		MSG_VOL_DOWN,\
							/*05*/		NO_MSG,\
							/*06*/		NO_MSG,\
							/*07*/		NO_MSG,\
							/*08*/		NO_MSG,\
							/*09*/		NO_MSG,\

#define ADKEY_REMOTE_CONTROLLER_LONG_UP \
							/*00*/		NO_MSG,\
							/*01*/		NO_MSG,\
							/*02*/		NO_MSG,\
							/*03*/		NO_MSG,\
							/*04*/		NO_MSG,\
							/*05*/		NO_MSG,\
							/*06*/		NO_MSG,\
							/*07*/		NO_MSG,\
							/*08*/		NO_MSG,\
							/*09*/		NO_MSG,\

#if (KEY_DOUBLE_CLICK_EN)
#define ADKEY_REMOTE_CONTROLLER_DOUBLE_KICK \
							/*00*/		NO_MSG,\
							/*01*/		NO_MSG,\
							/*02*/		NO_MSG,\
							/*03*/		NO_MSG,\
							/*04*/		NO_MSG,\
							/*05*/		NO_MSG,\
							/*06*/		NO_MSG,\
							/*07*/		NO_MSG,\
							/*08*/		NO_MSG,\
							/*09*/		NO_MSG,\

#define ADKEY_REMOTE_CONTROLLER_TRIPLE_KICK \
							/*00*/		NO_MSG,\
							/*01*/		NO_MSG,\
							/*02*/		NO_MSG,\
							/*03*/		NO_MSG,\
							/*04*/		NO_MSG,\
							/*05*/		NO_MSG,\
							/*06*/		NO_MSG,\
							/*07*/		NO_MSG,\
							/*08*/		NO_MSG,\
							/*09*/		NO_MSG,\


#endif

#define ADKEY_REMOTE_CONTROLLER_SHORT \
							/*00*/		NO_MSG,\
							/*01*/		NO_MSG,\
							/*02*/		NO_MSG,\
							/*03*/		NO_MSG,\
							/*04*/		NO_MSG,\
							/*05*/		NO_MSG,\
							/*06*/		NO_MSG,\
							/*07*/		NO_MSG,\
							/*08*/		NO_MSG,\
							/*09*/		NO_MSG,\

const u16 adkey_msg_remote_controller_table[][AD_KEY_MAX_NUM] = {
    /*短按*/		{ADKEY_REMOTE_CONTROLLER_SHORT},
    /*短按抬起*/	{ADKEY_REMOTE_CONTROLLER_SHORT_UP},
    /*长按*/		{ADKEY_REMOTE_CONTROLLER_LONG},
    /*连按*/		{ADKEY_REMOTE_CONTROLLER_HOLD},
    /*长按抬起*/	{ADKEY_REMOTE_CONTROLLER_LONG_UP},
#if (KEY_DOUBLE_CLICK_EN)
    /*双击*/		{ADKEY_REMOTE_CONTROLLER_DOUBLE_KICK},
    /*三击*/        {ADKEY_REMOTE_CONTROLLER_TRIPLE_KICK},
#endif
};
#endif
#if KEY_IO_EN
#define IOKEY_REMOTE_CONTROLLER_SHORT_UP \
							/*00*/		NO_MSG,\
							/*01*/		NO_MSG,\
							/*02*/		NO_MSG,\
							/*03*/		MSG_SPEAKER,\
							/*04*/		NO_MSG,\
							/*05*/		NO_MSG,\
							/*06*/		NO_MSG,\
							/*07*/		NO_MSG,\
							/*08*/		NO_MSG,\
							/*09*/		NO_MSG,\

#define IOKEY_REMOTE_CONTROLLER_LONG \
							/*00*/		NO_MSG,\
							/*01*/		NO_MSG,\
							/*02*/		NO_MSG,\
							/*03*/		NO_MSG,\
							/*04*/		NO_MSG,\
							/*05*/		NO_MSG,\
							/*06*/		NO_MSG,\
							/*07*/		NO_MSG,\
							/*08*/		NO_MSG,\
							/*09*/		NO_MSG,\

#define IOKEY_REMOTE_CONTROLLER_HOLD \
							/*00*/		NO_MSG,\
							/*01*/		NO_MSG,\
							/*02*/		NO_MSG,\
							/*03*/		NO_MSG,\
							/*04*/		NO_MSG,\
							/*05*/		NO_MSG,\
							/*06*/		NO_MSG,\
							/*07*/		NO_MSG,\
							/*08*/		NO_MSG,\
							/*09*/		NO_MSG,\

#define IOKEY_REMOTE_CONTROLLER_LONG_UP \
							/*00*/		NO_MSG,\
							/*01*/		NO_MSG,\
							/*02*/		NO_MSG,\
							/*03*/		NO_MSG,\
							/*04*/		NO_MSG,\
							/*05*/		NO_MSG,\
							/*06*/		NO_MSG,\
							/*07*/		NO_MSG,\
							/*08*/		NO_MSG,\
							/*09*/		NO_MSG,\

#if (KEY_DOUBLE_CLICK_EN)
#define IOKEY_REMOTE_CONTROLLER_DOUBLE_KICK \
							/*00*/		NO_MSG,\
							/*01*/		NO_MSG,\
							/*02*/		NO_MSG,\
							/*03*/		NO_MSG,\
							/*04*/		NO_MSG,\
							/*05*/		NO_MSG,\
							/*06*/		NO_MSG,\
							/*07*/		NO_MSG,\
							/*08*/		NO_MSG,\
							/*09*/		NO_MSG,\

#define IOKEY_REMOTE_CONTROLLER_TRIPLE_KICK \
							/*00*/		NO_MSG,\
							/*01*/		NO_MSG,\
							/*02*/		NO_MSG,\
							/*03*/		NO_MSG,\
							/*04*/		NO_MSG,\
							/*05*/		NO_MSG,\
							/*06*/		NO_MSG,\
							/*07*/		NO_MSG,\
							/*08*/		NO_MSG,\
							/*09*/		NO_MSG,\


#endif

#define IOKEY_REMOTE_CONTROLLER_SHORT \
							/*00*/		MSG_LEFT,\
							/*01*/		MSG_POWER,\
							/*02*/		MSG_RIGHT,\
							/*03*/		NO_MSG,\
							/*04*/		NO_MSG,\
							/*05*/		NO_MSG,\
							/*06*/		NO_MSG,\
							/*07*/		NO_MSG,\
							/*08*/		NO_MSG,\
							/*09*/		NO_MSG,\

const u16 iokey_msg_remote_controller_table[][IO_KEY_MAX_NUM] = {
    /*短按*/		{IOKEY_REMOTE_CONTROLLER_SHORT},
    /*短按抬起*/	{IOKEY_REMOTE_CONTROLLER_SHORT_UP},
    /*长按*/		{IOKEY_REMOTE_CONTROLLER_LONG},
    /*连按*/		{IOKEY_REMOTE_CONTROLLER_HOLD},
    /*长按抬起*/	{IOKEY_REMOTE_CONTROLLER_LONG_UP},
#if (KEY_DOUBLE_CLICK_EN)
    /*双击*/		{IOKEY_REMOTE_CONTROLLER_DOUBLE_KICK},
    /*三击*/        {IOKEY_REMOTE_CONTROLLER_TRIPLE_KICK},
#endif
};
#endif

#if KEY_MATRIX_EN
#define MATRIXKEY_REMOTE_CONTROLLER_SHORT_UP \
							/*00*/		MSG_MENU,\
							/*01*/		MSG_UP,\
							/*02*/		MSG_MUTE,\
							/*03*/		MSG_VOL_DOWN,\
							/*04*/		MSG_CONFIRM,\
							/*05*/		MSG_VOL_UP,\
							/*06*/		MSG_RETURN,\
							/*07*/		MSG_DOWN,\
							/*08*/		MSG_MAINPAGE,\
							/*09*/		NO_MSG,\

#define MATRIXKEY_REMOTE_CONTROLLER_LONG \
							/*00*/		NO_MSG,\
							/*01*/		NO_MSG,\
							/*02*/		NO_MSG,\
							/*03*/		MSG_VOL_DOWN,\
							/*04*/		NO_MSG,\
							/*05*/		MSG_VOL_UP,\
							/*06*/		NO_MSG,\
							/*07*/		NO_MSG,\
							/*08*/		NO_MSG,\
							/*09*/		NO_MSG,\

#define MATRIXKEY_REMOTE_CONTROLLER_HOLD \
							/*00*/		NO_MSG,\
							/*01*/		NO_MSG,\
							/*02*/		NO_MSG,\
							/*03*/		MSG_VOL_DOWN,\
							/*04*/		NO_MSG,\
							/*05*/		MSG_VOL_UP,\
							/*06*/		NO_MSG,\
							/*07*/		NO_MSG,\
							/*08*/		NO_MSG,\
							/*09*/		NO_MSG,\

#define MATRIXKEY_REMOTE_CONTROLLER_LONG_UP \
							/*00*/		NO_MSG,\
							/*01*/		NO_MSG,\
							/*02*/		NO_MSG,\
							/*03*/		NO_MSG,\
							/*04*/		NO_MSG,\
							/*05*/		NO_MSG,\
							/*06*/		NO_MSG,\
							/*07*/		NO_MSG,\
							/*08*/		NO_MSG,\
							/*09*/		NO_MSG,\

#if (KEY_DOUBLE_CLICK_EN)
#define MATRIXKEY_REMOTE_CONTROLLER_DOUBLE_KICK \
							/*00*/		NO_MSG,\
							/*01*/		NO_MSG,\
							/*02*/		NO_MSG,\
							/*03*/		NO_MSG,\
							/*04*/		NO_MSG,\
							/*05*/		NO_MSG,\
							/*06*/		NO_MSG,\
							/*07*/		NO_MSG,\
							/*08*/		NO_MSG,\
							/*09*/		NO_MSG,\

#define MATRIXKEY_REMOTE_CONTROLLER_TRIPLE_KICK \
							/*00*/		NO_MSG,\
							/*01*/		NO_MSG,\
							/*02*/		NO_MSG,\
							/*03*/		NO_MSG,\
							/*04*/		NO_MSG,\
							/*05*/		NO_MSG,\
							/*06*/		NO_MSG,\
							/*07*/		NO_MSG,\
							/*08*/		NO_MSG,\
							/*09*/		NO_MSG,\


#endif

#define MATRIXKEY_REMOTE_CONTROLLER_SHORT \
							/*00*/		NO_MSG,\
							/*01*/		NO_MSG,\
							/*02*/		NO_MSG,\
							/*03*/		NO_MSG,\
							/*04*/		NO_MSG,\
							/*05*/		NO_MSG,\
							/*06*/		NO_MSG,\
							/*07*/		NO_MSG,\
							/*08*/		NO_MSG,\
							/*09*/		NO_MSG,\

const u16 matrixkey_msg_remote_controller_table[][MATRIX_KEY_MAX_NUM] = {
    /*短按*/		{MATRIXKEY_REMOTE_CONTROLLER_SHORT},
    /*短按抬起*/	{MATRIXKEY_REMOTE_CONTROLLER_SHORT_UP},
    /*长按*/		{MATRIXKEY_REMOTE_CONTROLLER_LONG},
    /*连按*/		{MATRIXKEY_REMOTE_CONTROLLER_HOLD},
    /*长按抬起*/	{MATRIXKEY_REMOTE_CONTROLLER_LONG_UP},
#if (KEY_DOUBLE_CLICK_EN)
    /*双击*/		{MATRIXKEY_REMOTE_CONTROLLER_DOUBLE_KICK},
    /*三击*/        {MATRIXKEY_REMOTE_CONTROLLER_TRIPLE_KICK},
#endif
};
#endif

u16 rc_key_msg_filter(u8 key_status, u8 key_num, u8 key_type)
{
    u16 msg = NO_MSG;
    switch (key_type) {
#if KEY_AD_EN
    case KEY_TYPE_AD:
        msg = adkey_msg_remote_controller_table[key_status][key_num];
        break;
#endif
#if KEY_IO_EN
    case KEY_TYPE_IO:
        msg = iokey_msg_remote_controller_table[key_status][key_num];
        break;
#endif
#if KEY_MATRIX_EN
    case KEY_TYPE_MATRIX:
        msg = matrixkey_msg_remote_controller_table[key_status][key_num];
        break;
#endif
    }

#if SYS_TIMER_SOFTOFF
    if (msg != NO_MSG) {
        extern void set_softoff_countdown(u32 count_down);
        set_softoff_countdown(SYS_TIMER_SOFTOFF_TIME);
    }
#endif
    return msg;
}

