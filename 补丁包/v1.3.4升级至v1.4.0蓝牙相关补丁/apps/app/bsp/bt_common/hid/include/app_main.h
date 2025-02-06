#ifndef APP_MAIN_H
#define APP_MAIN_H

#include "list.h"

#define SYS_ALL_EVENT           0xffff
#define SYS_KEY_EVENT 			0x0001
#define SYS_TOUCH_EVENT 		0x0002
#define SYS_DEVICE_EVENT 		0x0004
#define SYS_NET_EVENT 		    0x0008
#define SYS_BT_EVENT 		    0x0010
#define SYS_IR_EVENT 		    0x0020
#define SYS_PBG_EVENT           0x0040
#define SYS_BT_AI_EVENT 		0x0080
#define SYS_AI_EVENT 		    0x0100
#define SYS_MATRIX_KEY_EVENT    0x0200
#define SYS_TOUCHPAD_EVENT      0x0400
#define SYS_ADT_EVENT      0x0800


#define DEVICE_EVENT_FROM_AT_UART      (('A' << 24) | ('T' << 16) | ('U' << 8) | '\0')
#define DEVICE_EVENT_FROM_CHARGE	   (('C' << 24) | ('H' << 16) | ('G' << 8) | '\0')
#define DEVICE_EVENT_FROM_POWER		   (('P' << 24) | ('O' << 16) | ('W' << 8) | '\0')
#define DEVICE_EVENT_FROM_CI_UART	   (('C' << 24) | ('I' << 16) | ('U' << 8) | '\0')
#define DEVICE_EVENT_FROM_CI_TWS 	   (('C' << 24) | ('I' << 16) | ('T' << 8) | '\0')
#define DEVICE_EVENT_CHARGE_STORE	   (('S' << 24) | ('T' << 16) | ('O' << 8) | '\0')
#define DEVICE_EVENT_UMIDIGI_CHARGE_STORE	   (('Q' << 24) | ('C' << 16) | ('Y' << 8) | '\0')
#define DEVICE_EVENT_TEST_BOX    	   (('T' << 24) | ('S' << 16) | ('B' << 8) | '\0')
#define DEVICE_EVENT_FROM_TONE		   (('T' << 24) | ('N' << 16) | ('E' << 8) | '\0')
#define DEVICE_EVENT_FROM_FM		   (('F' << 24) | ('M' << 16) | ('\0'<< 8) | '\0')
#define KEY_EVENT_FROM_TWS             (('T' << 24) | ('W' << 16) | ('S' << 8) | '\0')
#define SYS_BT_EVENT_FROM_TWS          (('T' << 24) | ('W' << 16) | ('S' << 8) | '\0')
#define DEVICE_EVENT_FROM_LINEIN	   (('A' << 24) | ('U' << 16) | ('X' << 8) | '\0')
#define DRIVER_EVENT_FROM_SD0          (('S' << 24) | ('D' << 16) | ('0' << 8) | '\0')
#define DRIVER_EVENT_FROM_SD1          (('S' << 24) | ('D' << 16) | ('1' << 8) | '\0')
#define DRIVER_EVENT_FROM_SD2          (('S' << 24) | ('D' << 16) | ('2' << 8) | '\0')
#define DEVICE_EVENT_FROM_MUSIC		   (('M' << 24) | ('S' << 16) | ('C' << 8) | '\0')
#define DEVICE_EVENT_FROM_USB_HOST     (('U' << 24) | ('H' << 16) | '\0' | '\0')
#define DEVICE_EVENT_FROM_OTG          (('O' << 24) | ('T' << 16) | ('G' << 8) | '\0')
#define DEVICE_EVENT_FROM_PC		   (('P' << 24) | ('C' << 16) | '\0' | '\0')
#define DEVICE_EVENT_FROM_UAC          (('U' << 24) | ('A' << 16) | ('C' << 8) | '\0')
#define DEVICE_EVENT_FROM_ALM		   (('A' << 24) | ('L' << 16) | ('M' << 8) | '\0')
#define SYS_BT_EVENT_TYPE_CON_STATUS   (('C' << 24) | ('O' << 16) | ('N' << 8) | '\0')
#define SYS_BT_EVENT_TYPE_HCI_STATUS   (('H' << 24) | ('C' << 16) | ('I' << 8) | '\0')
#define SYS_BT_EVENT_BLE_STATUS        (('B' << 24) | ('L' << 16) | ('E' << 8) | '\0')
#define SYS_BT_EVENT_FORM_COMMON       (('C' << 24) | ('M' << 16) | ('M' << 8) | '\0')
#define DEVICE_EVENT_FROM_KEY		   (('K' << 24) | ('E' << 16) | ('Y' << 8) | '\0')
#define DEVICE_EVENT_FROM_CUSTOM	   (('C' << 24) | ('S' << 16) | ('T' << 8) | '\0')
#define SYS_BT_AI_EVENT_TYPE_STATUS    (('B' << 24) | ('A' << 16) | ('I' << 8) | '\0')
#define DEVICE_EVENT_FROM_UART_RX_OVERFLOW		(('U' << 24) | ('R' << 16) | ('F' << 8) | '\0')
#define DEVICE_EVENT_FROM_UART_RX_OUTTIME		(('U' << 24) | ('R' << 16) | ('T' << 8) | '\0')
#define DEVICE_EVENT_FROM_DAC		   (('D' << 24) | ('A' << 16) | ('C' << 8) | '\0')
#define SYS_EVENT_FROM_CTRLER          (('C' << 24) | ('T' << 16) | ('R' << 8) | '\0')
#define SYS_EVENT_FROM_RECORD          (('R' << 24) | ('E' << 16) | ('C' << 8) | '\0')
#define DEVICE_EVENT_FROM_ENDLESS_LOOP_DEBUG          (('E' << 24) | ('L' << 16) | ('D' << 8) | '\0')
#define DEVICE_EVENT_FROM_EARTCH	   (('E' << 24) | ('T' << 16) | ('H' << 8) | '\0')
#define DEVICE_EVENT_ONLINE_DATA	   (('O' << 24) | ('L' << 16) | ('D' << 8) | '\0')
#define SYS_BT_EVENT_FROM_KEY       (('K' << 24) | ('E' << 16) | ('Y' << 8) | '\0')
#define SYS_BT_EVENT_FORM_SELF  (('S' << 24) | ('E' << 16) | ('F' << 8) | '\0')
#define DEVICE_EVENT_FROM_ANC   	   (('A' << 24) | ('N' << 16) | ('C' << 8) | '\0')
#define SYS_BT_EVENT_FORM_AT          (('I' << 24) | ('A' << 16) | ('T' << 8) | '\0')
#define DEVICE_EVENT_FROM_ADAPTER      (('A' << 24) | ('D' << 16) | ('A' << 8) | '\0')
#define DEVICE_EVENT_FROM_BOARD_UART   (('B' << 24) | ('D' << 16) | ('U' << 8) | '\0')

enum app_state {
    APP_STA_CREATE,
    APP_STA_START,
    APP_STA_PAUSE,
    APP_STA_RESUME,
    APP_STA_STOP,
    APP_STA_DESTROY,
};


enum {
    KEY_EVENT_CLICK,
    KEY_EVENT_LONG,
    KEY_EVENT_HOLD,
    KEY_EVENT_UP,
    KEY_EVENT_DOUBLE_CLICK,
    KEY_EVENT_TRIPLE_CLICK,
    KEY_EVENT_FOURTH_CLICK,
    KEY_EVENT_FIRTH_CLICK,
    KEY_EVENT_USER,
    KEY_EVENT_MAX,
};

struct intent {
    const char *name;
    int action;
    const char *data;
    u32 exdata;
};

struct key_event {
    u8 init;
    u8 type;
    u16 event;
    u32 value;
    u32 tmr;
};

struct ir_event {
    u8 event;
};

struct msg_event {
    u8 event;
    u8 value;
};

// #if EVENT_TOUCH_ENABLE_CONFIG
// struct touch_event {
//     u8 event;
//     struct position pos;
// };
// #endif

struct device_event {
    u8 event;
    int value;
};

struct notify_433 {
    u8 event;
    u8 *packet;
    u8 size;
};

struct notify_nfc {
    u8 event;
    u8 *packet;
    u8 size;
};

struct notify_onepa {
    u8 event;
    u8 *packet;
    u8 size;
};

struct chargestore_event {
    u8 event;
    u8 *packet;
    u8 size;
};

struct testbox_event {
    u8 event;
    u8 *packet;
    u8 size;
};

struct dg_ota_event {
    u8 event;
    u8 *packet;
    u8 size;
};

struct umidigi_chargestore_event {
    u8 event;
    u8 value;
};

struct ancbox_event {
    u8 event;
    u32 value;
};

struct net_event {
    u8 event;
    u8 value;
};
struct upper_bt_event {
    u8 event;
    u8 args[7];
    u32 value;
};

struct axis_event {
    u8 event;
    s16 x;
    s16 y;
};

struct codesw_event {
    u8 event;
    s8 value;
};

struct pbg_event {
    u8 event;
    u8 args[3];
};

struct adt_event {
    u8 event;
    u8 args[3];
};

struct uart_event {
    void *ut_bus;
};

struct uart_cmd_event {
    u8 type;
    u8 cmd;
};

struct ai_event {
    u32 value;
};

struct ear_event {
    u8 value;
};

struct rcsp_event {
    u8 event;
    u8 args[6];
    u8 size;
};

struct chargebox_event {
    u8 event;
};

struct matrix_key_event {
    u16 args[6];            //最多推6个按键出来，如果需要推多个按键需要自行修改，每个u16 低八位标识row 高八位标识col
    u8 *map;
};

struct touchpad_event {
    u8 gesture_event;       //手势事件
    s8 x;
    s8 y;
};

struct sys_event {
    u16 type;
    u8 consumed;
    void *arg;
    union {
        struct key_event key;
        struct axis_event axis;
        struct codesw_event codesw;
// #if EVENT_TOUCH_ENABLE_CONFIG
//         struct touch_event 	touch;
// #endif
        struct device_event dev;
        struct notify_433   nf4;
        struct notify_nfc   nfn;
        struct notify_onepa nfo;
        struct net_event 	net;
        struct upper_bt_event 	bt;
        struct msg_event 	msg;
        struct chargestore_event chargestore;
        struct ir_event     ir;
        struct pbg_event    pbg;
        struct uart_event	uart;
        struct uart_cmd_event	uart_cmd;
        struct ai_event     ai;
        struct ear_event    ear;
        struct rcsp_event	rcsp;
        struct chargebox_event chargebox;
        struct dg_ota_event dg_ota;
        struct testbox_event testbox;
        struct umidigi_chargestore_event umidigi_chargestore;
        struct ancbox_event ancbox;
        struct matrix_key_event  matrix_key;
        struct touchpad_event touchpad;
        struct adt_event    adt;
    } u;
};




struct static_event_handler {
    int event_type;
    void (*handler)(struct sys_event *);
};


typedef struct _APP_VAR {
    s8 music_volume;
    s8 call_volume;
    s8 wtone_volume;
    u8 opid_play_vol_sync;
    u8 aec_dac_gain;
    u8 aec_mic_gain;
    u8 rf_power;
    u8 goto_poweroff_flag;
    u8 goto_poweroff_cnt;
    u8 play_poweron_tone;
    u8 remote_dev_company;
    u8 siri_stu;
    int auto_stop_page_scan_timer;     //用于1拖2时，有一台连接上后，超过三分钟自动关闭Page Scan
    volatile int auto_shut_down_timer;
    volatile int wait_exit_timer;
    u16 auto_off_time;
    u16 warning_tone_v;
    u16 poweroff_tone_v;
    u32 start_time;
    s8  usb_mic_gain;
} APP_VAR;

extern APP_VAR app_var;

struct application {
    u8 	state;
    int action;
    char *data;
    const char *name;
    struct list_head entry;
    void *private_data;
    const struct application_operation *ops;
};

struct application_operation {
    int (*state_machine)(struct application *, enum app_state, struct intent *);
    int (*event_handler)(struct application *, struct sys_event *);
};

#define REGISTER_APPLICATION(ops) \
        const struct application ops SEC_USED(.app_main)

extern const struct application app_main_begin[];
extern const struct application app_main_end[];

#define list_for_each_app_main(p) \
    for (p = app_main_begin; p < app_main_end; p++)

#define init_intent(it) \
	do { \
		(it)->name = NULL; \
		(it)->action= 0; \
		(it)->data = NULL; \
		(it)->exdata = 0; \
	}while (0)


#endif
