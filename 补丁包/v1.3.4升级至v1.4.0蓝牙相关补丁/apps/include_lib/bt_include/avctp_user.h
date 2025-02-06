#ifndef __AVCTP_USER_H__
#define __AVCTP_USER_H__


#include "typedef.h"
#include "btstack_typedef.h"



///***注意：该文件的枚举与库编译密切相关，主要是给用户提供调用所用。用户不能自己在中间添加值。*/
////----user (command) codes----////
typedef enum {
    /*
    使用bt_cmd_prepare(USER_CMD_TYPE cmd,u16 param_len,u8 *param)发送命令
    //返回0表支持参数个数正确，返回1表不支持，2是参数错误
    要三个参数，没参数说明的命令参数param_len传0，param传NULL
    例子A、USER_CTRL_HFP_CALL_SET_VOLUME命令需要1个参数的使用例子：
    u8 vol = 8;
    bt_cmd_prepare(USER_CTRL_HFP_CALL_SET_VOLUME,1, &vol);

    例子B、USER_CTRL_HFP_DIAL_NUMBER 参数要用数组先存起来，param_len是号码长度，param可传参数数组指针，
    user_val->income_phone_num已经存好号码
    bt_cmd_prepare(USER_CTRL_HFP_DIAL_NUMBER,user_val->phone_num_len,user_val->income_phone_num);

    */

    //链路操作部分
    //回连,使用的是VM的地址，一般按键操作不使用该接口
    USER_CTRL_START_CONNECTION        =  0x00,
    //通过地址去连接，如果知道地址想去连接使用该接口
    USER_CTRL_START_CONNEC_VIA_ADDR,
    //通过指定地址手动回连，该地址是最后一个断开设备的地址
    USER_CTRL_START_CONNEC_VIA_ADDR_MANUALLY,
    //通过地址去连接spp，如果知道地址想去连接使用该接口
    USER_CTRL_START_CONNEC_SPP_VIA_ADDR,

    //断开连接，断开当前所有蓝牙连接,可以加地址进行指定断开
    USER_CTRL_DISCONNECTION_HCI,

    //取消链接
    USER_CTRL_CONNECTION_CANCEL,

    //读取远端名字
    USER_CTRL_READ_REMOTE_NAME,
    //连接或断开SCO或esco,选择这个命令会自动判断要断开还是连接sco
    USER_CTRL_SCO_LINK,
    //连接SCO或esco
    USER_CTRL_CONN_SCO,
    //断开sco或esco
    USER_CTRL_DISCONN_SCO,
    //断开并配置拒绝远端ESCO连接
    USER_CTRL_DISCONN_REJECT_SCO,
    //断开SDP，一般按键操作不使用该接口
    USER_CTRL_DISCONN_SDP_MASTER,

    //直接发底层断开，跳过协议的断开
    USER_CTRL_DETACH,

    //关闭蓝牙可发现
    USER_CTRL_WRITE_SCAN_DISABLE,
    //打开蓝牙可发现
    USER_CTRL_WRITE_SCAN_ENABLE,
    //关闭蓝牙可连接
    USER_CTRL_WRITE_CONN_DISABLE,
    //打开蓝牙可连接
    USER_CTRL_WRITE_CONN_ENABLE,
    //控制蓝牙搜索，需要搜索附件设备做功能的连续说明情况在补充完善功能
    USER_CTRL_SEARCH_DEVICE,
    //取消搜索
    USER_CTRL_INQUIRY_CANCEL,
    //取消配对
    USER_CTRL_PAGE_CANCEL,
    ///进入sniff模式，一般按键操作不使用该接口
    USER_CTRL_SNIFF_IN,
    USER_CTRL_SNIFF_EXIT,
    USER_CTRL_SNIFF_CLEAR_CNT,
    USER_CTRL_ALL_SNIFF_EXIT,

    //链路操作层接口位置预留
    USER_CTRL_HCI_FUNCTION1,

    //hfp链路部分
    //控制打电话音量，注意可能有些手机进度条有变化音量大小没变化，同步要设置样机DAC音量
    /*跟电话音量操作有关的操作最终都执行回调函数call_vol_change*/
    USER_CTRL_HFP_CMD_BEGIN        = 0x20,
    USER_CTRL_HFP_CMD_CONN,           /* 做HFP 连接 */
    USER_CTRL_HFP_CALL_VOLUME_UP,       /*音量加1，手机可以同步显示*/
    USER_CTRL_HFP_CALL_VOLUME_DOWN,      /*音量减1，手机可以同步显示*/
    USER_CTRL_HFP_CALL_SET_VOLUME,   /*设置固定值，手机可以同步显示，需要传1个音量值*/
    USER_CTRL_HFP_CALL_GET_VOLUME,  /*获取音量，默认从call_vol_change返回*/

    //来电接听电话
    USER_CTRL_HFP_CALL_ANSWER,
    //挂断电话
    USER_CTRL_HFP_CALL_HANGUP,
    //回拨上一个打出电话
    USER_CTRL_HFP_CALL_LAST_NO,
    //获取当前通话电话号码
    USER_CTRL_HFP_CALL_CURRENT,
    //通话过程中根据提示输入控制
    /*例子
    char num = '1';
    bt_cmd_prepare(USER_CTRL_HFP_DTMF_TONES,1,(u8 *)&num);
    */
    //发送打电话时的信号选择DTMF tones ,有一个参数，参数支持{0-9, *, #, A, B, C, D}
    USER_CTRL_HFP_DTMF_TONES,
    //根据电话号码拨号
    /**USER_CTRL_HFP_DIAL_NUMBER命令有参数，参数要用数组先存起来，
    param_len是号码长度，param可传参数数组指针*/
    USER_CTRL_HFP_DIAL_NUMBER,
    //*控制siri状态*//*可以注册回调函数获取返回值*/
    USER_CTRL_HFP_GET_SIRI_STATUS,
    //*开启siri*/
    USER_CTRL_HFP_GET_SIRI_OPEN,
    //*关闭siri,一般说完话好像自动关闭了,如果要提前终止可调用*/
    USER_CTRL_HFP_GET_SIRI_CLOSE,
    /*获取手机的日期和时间，苹果可以，一般安卓机好像都不行*/
    USER_CTRL_HFP_GET_PHONE_DATE_TIME,
    USER_CTRL_HFP_CMD_SEND_BIA,
    /*获取手机厂商的命令 */
    USER_CTRL_HFP_CMD_GET_MANUFACTURER,
    /*更新当前的电量给手机*/
    USER_CTRL_HFP_CMD_UPDATE_BATTARY,
    //三方通话操作
    //应答
    //挂断当前去听另一个（未接听或者在保留状态都可以）
    USER_CTRL_HFP_THREE_WAY_ANSWER1,
    //保留当前去接听, 或者用于两个通话的切换
    USER_CTRL_HFP_THREE_WAY_ANSWER2,
    USER_CTRL_HFP_THREE_WAY_ANSWER1X,    //目前没有用
    USER_CTRL_HFP_THREE_WAY_ANSWER2X,    //目前没有用
    //可以发完USER_CTRL_HFP_THREE_WAY_ANSWER2,又发ANSWER3，自己看看效果
    USER_CTRL_HFP_THREE_WAY_ANSWER3,
    //拒听
    USER_CTRL_HFP_THREE_WAY_REJECT,     //拒绝后台来电
    USER_CTRL_HFP_DISCONNECT,           //断开HFP连接

    //bt_cmd_prepare(USER_CTRL_HFP_SEND_USER_AT_CMD,data_ptr,len);
    USER_CTRL_HFP_SEND_USER_AT_CMD,        //用户发送自己定义的AT命令
    USER_CTRL_HFP_CMD_FUNCTION1,            //预留HFP命令位置
    USER_CTRL_HFP_CMD_FUNCTION2,            //预留HFP命令位置
    USER_CTRL_HFP_CMD_END,

    //音乐控制部分
    USER_CTRL_AVCTP_CMD_BEGIN       = 0x40,
    //只会发pause命令
    USER_CTRL_AVCTP_PAUSE_MUSIC,
    //音乐播放,会根据协议栈记录的状态发PLAY或者PAUSE
    USER_CTRL_AVCTP_OPID_PLAY,
    //音乐暂停,会根据协议栈记录的状态发PLAY或者PAUSE
    USER_CTRL_AVCTP_OPID_PAUSE,
    //音乐停止
    USER_CTRL_AVCTP_OPID_STOP,
    //音乐下一首
    USER_CTRL_AVCTP_OPID_NEXT,
    //音乐上一首
    USER_CTRL_AVCTP_OPID_PREV,
    //音乐快进
    USER_CTRL_AVCTP_OPID_FORWARD,
    //音乐快退
    USER_CTRL_AVCTP_OPID_REWIND,
    //音乐循环模式
    USER_CTRL_AVCTP_OPID_REPEAT_MODE,
    USER_CTRL_AVCTP_OPID_SHUFFLE_MODE,
    //获取播放歌曲总时间和当前时间接口
    USER_CTRL_AVCTP_OPID_GET_PLAY_TIME,
    //获取当前音乐的一些信息.
    //通过bt_music_info_handle_register注册的接口返回
    USER_CTRL_AVCTP_OPID_GET_MUSIC_INFO,

    //同步音量接口
    USER_CTRL_AVCTP_OPID_SEND_VOL,
//    //AVCTP断开，是音乐控制链路，
    USER_CTRL_AVCTP_DISCONNECT,
//    //AVCTP连接，是音乐控制链路，
    USER_CTRL_AVCTP_CONN,

    USER_CTRL_AVCTP_CMD_FUNCTION1,            //预留AVCTP命令位置
    USER_CTRL_AVCTP_CMD_FUNCTION2,            //预留AVCTP命令位置
    USER_CTRL_AVCTP_CMD_END,

    //高级音频部分
    USER_CTRL_A2DP_CMD_BEGIN        = 0x60,
    //有判断条件的，回连过程连接高级音频，避免手机连也自动发起连接，一般按键操作不使用该接口
    USER_CTRL_AUTO_CONN_A2DP,
    //连接高级音频，回来最后一个断开设备的地址
    USER_CTRL_CONN_A2DP,
    //断开高级音频，只断开高级音频链路，如果有电话还会保留
    USER_CTRL_DISCONN_A2DP,
    //maybe BQB test will use
    USER_CTRL_A2DP_CMD_START					,
    USER_CTRL_A2DP_CMD_CLOSE				,
    USER_CTRL_A2DP_CMD_SUSPEND					,
    USER_CTRL_A2DP_CMD_GET_CONFIGURATION		,
    USER_CTRL_A2DP_CMD_ABORT					,
    USER_CTRL_A2DP_CMD_DELAY_REPORT,              /*delay report cmd*/
    /**音乐音量同步接口，自动选择通过HID还是AVRCP来发送*/
    USER_CTRL_CMD_SYNC_VOL_INC,
    /**音乐音量同步接口，自动选择通过HID还是AVRCP来发送*/
    USER_CTRL_CMD_SYNC_VOL_DEC,
    USER_CTRL_A2DP_CMD_FUNCTION1,            //预留A2DP命令位置
    USER_CTRL_A2DP_CMD_FUNCTION2,            //预留A2DP命令位置
    USER_CTRL_A2DP_CMD_END,

    ///*hid操作定义*/
    USER_CTRL_HID_CMD_BEGIN         = 0x70,
    //按键连接
    USER_CTRL_HID_CONN,
//    //只发一个按键，安卓手机使用
    USER_CTRL_HID_ANDROID,
    //只发一个按键，苹果和部分安卓手机适用
    USER_CTRL_HID_IOS,
//    //发两个拍照按键
    USER_CTRL_HID_BOTH,
    //HID断开
    USER_CTRL_HID_DISCONNECT,
    //Home Key,apply to IOS and Android
    USER_CTRL_HID_HOME				,
    //Return Key,only support Android
    USER_CTRL_HID_RETURN			,
    //LeftArrow Key
    USER_CTRL_HID_LEFTARROW			,
    //RightArrow Key
    USER_CTRL_HID_RIGHTARROW		,
    //Volume Up
    USER_CTRL_HID_VOL_UP			,
    //Volume Down
    USER_CTRL_HID_VOL_DOWN			,
    //提高接口给用户层发送HID数据
    USER_CTRL_HID_SEND_DATA			,

    USER_CTRL_HID_CMD_END,

    ///蓝牙串口发送命令
    USER_CTRL_SPP_CMD_BEGIN         = 0x80,
    /**USER_CTRL_SPP_SEND_DATA命令有参数，参数会先存起来，
      param_len是数据长度，param发送数据指针
      返回0,表示准备成功，其它值参考底下的错误表*/
    USER_CTRL_SPP_SEND_DATA, //len <= 512
    USER_CTRL_SPP_TRY_SEND_DATA,//
    USER_CTRL_SPP_UPDATA_DATA,
    //serial port profile disconnect command
    USER_CTRL_SPP_DISCONNECT,

    //支持多串口功能的时候发送接口,例如支持一台手机的多个串口软件同时连接
    USER_CTRL_SPP_SEND_DATA_ID_A,
    USER_CTRL_SPP_SEND_DATA_ID_B,
    USER_CTRL_SPP_SEND_DATA_ID_C,
    USER_CTRL_SPP_SEND_DATA_ID_D,
    USER_CTRL_SPP_SEND_DATA_ID_E,
    USER_CTRL_SPP_SEND_DATA_ID_F,
    USER_CTRL_SPP_SEND_DATA_ID_G,

    USER_CTRL_SPP_CMD_END,

    ///IAP发送命令,当不需要支持IAP时可配置第二路SPP
    USER_CTRL_IAP_CMD_BEGIN        = 0x90,
    /**USER_CTRL_IAP_SEND_DATA命令有参数，参数会先存起来，
      param_len是数据长度，param发送数据指针
      返回0,表示准备成功，其它值参考底下的错误表*/
    USER_CTRL_IAP_SEND_DATA, //len <= 512
    //serial port profile disconnect command
    USER_CTRL_IAP_DISCONNECT,
    USER_CTRL_IAP_CMD_END,

///pbg发送命令
    USER_CTRL_PBG_CMD_BEGIN       = 0xA0,
    USER_CTRL_PBG_SEND_DATA,//len <= 512
    USER_CTRL_PBG_TRY_SEND_DATA,//
    USER_CTRL_PBG_CMD_END,

///adt 发送命令
    USER_CTRL_ADT_CMD_BEGIN       = 0xB0,
    USER_CTRL_ADT_CONNECT,
    USER_CTRL_ADT_KEY_MIC_OPEN,
    USER_CTRL_ADT_SEND_DATA,//len <= 512
    USER_CTRL_ADT_TRY_SEND_DATA,//
    USER_CTRL_ADT_CMD_END,

    ///蓝牙电话本功能发送命令
    USER_CTRL_PBAP_CMD_BEGIN      = 0xC0,
    //电话本功能读取通话记录的前n条
    USER_CTRL_PBAP_READ_PART,
    //电话本功能读全部记录
    USER_CTRL_PBAP_READ_ALL,
    //电话本功能中断读取记录
    USER_CTRL_PBAP_STOP_READING,

    USER_CTRL_PBAP_CMD_END,

    //新增一个HSP的操作接口
    USER_CTRL_HSP_CMD_BEGIN        = 0xD0,
    USER_CTRL_HSP_CONNECT,                   //断开HSP连接
    USER_CTRL_HSP_DISCONNECT,                   //断开HSP连接
    USER_CTRL_HSP_SPEAK_VOLUME_UP,       /*音量加1*/
    USER_CTRL_HSP_SPEAK_VOLUME_DOWN,      /*音量减*/
    USER_CTRL_HSP_MIC_VOLUME_UP,       /*mic音量加1*/
    USER_CTRL_HSP_MIC_VOLUME_DOWN,      /*mic音量减1*/
    USER_CTRL_HSP_CKPD_CMD,             /*hsp 接听*/
    USER_CTRL_HSP_CMD_END,


    //MAP功能发送命令
    USER_CTRL_MAP_CMD_BEGIN       = 0xE0,
    //MAP读取时间
    USER_CTRL_MAP_READ_TIME,
    //MAP读取未读短信
    USER_CTRL_MAP_READ_INDOX,
    //MAP读取已读短信
    USER_CTRL_MAP_READ_OUTDOX,
    //MAP读取已发读短信
    USER_CTRL_MAP_READ_SENT,
    //MAP读取删除短信
    USER_CTRL_MAP_READ_DELETED,
    //MAP读取草稿箱短信
    USER_CTRL_MAP_READ_DRAFT,
    //MAP停止读取
    USER_CTRL_MAP_STOP_READING,
    USER_CTRL_MAP_CMD_END,

    //PAN功能发送命令
    USER_CTRL_PAN_CMD_BEGIN       = 0xF0,
    USER_CTRL_PAN_CONNECT,
    USER_CTRL_PAN_SEND_DATA,
    USER_CTRL_PAN_CMD_END,


    //蓝牙其他操作
    //蓝牙关闭
    USER_CTRL_POWER_OFF             = 0x100,
    //蓝牙开启
    USER_CTRL_POWER_ON,
    //几个蓝牙连接数据库的操作接口
    //回连VM中最新一个设备记忆
    USER_CTRL_CONNECT_LAST_REMOTE_INFO,
    //删除最新的一个设备记忆
    USER_CTRL_DEL_LAST_REMOTE_INFO,
    //删除所有设备记忆
    USER_CTRL_DEL_ALL_REMOTE_INFO,
    //测试盒测试的时候用来发操作消息
    USER_CTRL_TEST_KEY,
    //自定义LMP的数据发送,
    //通过重新定义weak函数void  bt_get_uesr_info(u32 info)获取值;
    USER_CTRL_SEND_USER_INFO,
    //加密流程需要输入数字的接口
    USER_CTRL_KEYPRESS,
    //加密流程本地需要按键确认继续连接的接口
    USER_CTRL_PAIR,
    //发送跳频数据的接口，长度是10个byte
    USER_CTRL_AFH_CHANNEL,
    //蓝牙库里面有些流程需要建立定时查询的接口
    USER_CTRL_HALF_SEC_LOOP_CREATE,
    //蓝牙库里面有些流程需要删除定时查询的接口
    USER_CTRL_HALF_SEC_LOOP_DEL,
    /*单独HID和普通蓝牙模式的切换接口,音箱SDK才有完整流程*/
    USER_CTRL_CMD_CHANGE_PROFILE_MODE,
    //唤醒协议栈跑一次
    USER_CTRL_CMD_RESUME_STACK,

    USER_CTRL_LAST
} USER_CMD_TYPE;


////----反馈给客户使用的状态----////
typedef enum {
    /*下面是一些即时反馈的状态，无法重复获取的状态*/
    BT_STATUS_POWER_ON   = 1,   /*上电*/
    BT_STATUS_POWER_OFF  = 2,
    BT_STATUS_INIT_OK,          /*初始化完成*/
    BT_STATUS_EXIT_OK,          /*蓝牙退出完成*/
    BT_STATUS_START_CONNECTED,        /*开始连接*/
    BT_STATUS_FIRST_CONNECTED,        /*连接成功*/
    BT_STATUS_SECOND_CONNECTED,        /*连接成功*/
    BT_STATUS_ENCRY_COMPLETE,        /*加密完成*/
    BT_STATUS_FIRST_DISCONNECT,       /*断开连接*/
    BT_STATUS_SECOND_DISCONNECT,        /*断开连接*/
    BT_STATUS_PHONE_INCOME,     /*来电*/
    BT_STATUS_PHONE_NUMBER,     /*来电话号码*/
    BT_STATUS_PHONE_MANUFACTURER,     /*获取手机的厂商*///13

    BT_STATUS_PHONE_OUT,        /*打出电话*/
    BT_STATUS_PHONE_ACTIVE,     /*接通电话*/
    BT_STATUS_PHONE_HANGUP,     /*挂断电话*/
    BT_STATUS_BEGIN_AUTO_CON,   /*发起回连*/
    BT_STATUS_MUSIC_SOUND_COME, /*库中加入auto mute判断音乐播放开始*/
    BT_STATUS_MUSIC_SOUND_GO,   /*库中加入auto mute判断音乐播放暂停*/
    BT_STATUS_RESUME,           /*后台有效，手动切回蓝牙*/
    BT_STATUS_RESUME_BTSTACK,   /*后台有效，后台时来电切回蓝牙*/
    BT_STATUS_SUSPEND,          /*蓝牙挂起，退出蓝牙*/
    BT_STATUS_LAST_CALL_TYPE_CHANGE,    /*最后拨打电话的类型，只区分打入和打出两种状态*///23

    BT_STATUS_CALL_VOL_CHANGE,     /*通话过程中设置音量会产生这个状态变化*/
    BT_STATUS_SCO_STATUS_CHANGE,    /*当esco/sco连接或者断开时会产生这个状态变化*/
    BT_STATUS_CONNECT_WITHOUT_LINKKEY,   /*判断是首次连接还是配对后的连接，主要依据要不要简易配对或者pin code*/
    BT_STATUS_PHONE_BATTERY_CHANGE,     /*电话电量变化，该状态仅6个等级，0-5*/
    BT_STATUS_RECONNECT_LINKKEY_LOST,     /*回连时发现linkkey丢失了，即手机取消配对了*/
    BT_STATUS_RECONN_OR_CONN,       /*回连成功还是被连接*/
    BT_STATUS_BT_TEST_BOX_CMD,              /*蓝牙收到测试盒消息。1-升级，2-fast test*/
    BT_STATUS_BT_TWS_CONNECT_CMD,
    BT_STATUS_SNIFF_STATE_UPDATE,              /*SNIFF STATE UPDATE*/
    BT_STATUS_TONE_BY_FILE_NAME, /*直接使用文件名播放提示音*/

    BT_STATUS_PHONE_DATE_AND_TIME,   /*获取到手机的时间和日期，注意会有兼容性问题*/
    BT_STATUS_INBAND_RINGTONE,
    BT_STATUS_VOICE_RECOGNITION,
    BT_STATUS_SIRI_OPEN,
    BT_STATUS_SIRI_CLOSE,
    BT_STATUS_SIRI_GET_STATE,
    BT_STATUS_AVRCP_INCOME_OPID,     /*收到远端设备发过来的AVRCP命令*/
    BT_STATUS_AVRCP_VOL_CHANGE,     /*协议栈更新音量值出来*/
    BT_STATUS_HFP_SERVICE_LEVEL_CONNECTION_OK,
    BT_STATUS_CONN_A2DP_CH,
    BT_STATUS_DISCON_A2DP_CH,
    BT_STATUS_CONN_HFP_CH,
    BT_STATUS_DISCON_HFP_CH,
    BT_STATUS_INQUIRY_TIMEOUT,
    /*下面是1个持续的状态，是get_stereo_bt_connect_status获取*/

    /*下面是6个持续的状态，是bt_get_connect_status()获取*/
    BT_STATUS_INITING,          /*正在初始化*/
    BT_STATUS_WAITINT_CONN,     /*等待连接*/
    BT_STATUS_AUTO_CONNECTINT,  /*正在回连*/
    BT_STATUS_CONNECTING,       /*已连接，没有电话和音乐在活动*/
    BT_STATUS_TAKEING_PHONE,    /*正在电话*/
    BT_STATUS_PLAYING_MUSIC,    /*正在音乐*/
    BT_STATUS_A2DP_MEDIA_START,
    BT_STATUS_A2DP_MEDIA_STOP,
    BT_STATUS_AVDTP_START,
    BT_STATUS_SCO_CONNECTION_REQ,
    BT_STATUS_SCO_DISCON,


    BT_STATUS_BROADCAST_STATE,/*braoadcaset中*/
    BT_STATUS_DONGLE_SPEAK_MIC,

    BT_STATUS_TRIM_OVER,        /*测试盒TRIM完成*/
} STATUS_FOR_USER;

typedef enum {
    BT_CALL_BATTERY_CHG = 0, //电池电量改变
    BT_CALL_SIGNAL_CHG,      //网络信号改变
    BT_CALL_INCOMING,   //电话打入
    BT_CALL_OUTGOING,   //电话打出
    BT_CALL_ACTIVE,     //接通电话
    BT_CALL_HANGUP,      //电话挂断
    BT_CALL_ALERT,       //远端reach
    BT_SIRI_STATE,       //SIRI状态
    BT_CALL_VOL_CHANGED,
} BT_CALL_IND_STA;

typedef enum {
    BT_MUSIC_STATUS_IDLE = 0,
    BT_MUSIC_STATUS_STARTING,
    BT_MUSIC_STATUS_SUSPENDING,
} BT_MUSIC_STATE;  //音乐状态

typedef enum {
    BT_ESCO_STATUS_CLOSE = 0,
    BT_ESCO_STATUS_OPEN,
} BT_ESCO_STATE;  //esoc状态

//bt_cmd_prepare_for_addr和bt_cmd_prepare
//返回值参考
enum {
    CMD_PREPARE_ERR_CODE_OK            = 0,
    CMD_PREPARE_ERR_CODE_NOT_INIT,
    CMD_PREPARE_ERR_CODE_NO_MALLOC,
    CMD_PREPARE_ERR_CODE_PARAM_INVALID,
    CMD_PREPARE_ERR_CODE_TOO_FAST,
    CMD_PREPARE_ERR_CODE_CONN_MAX,
    CMD_PREPARE_ERR_CODE_CMD_MAX_NUM,
    CMD_PREPARE_ERR_CODE_DATA_MAX_NUM,
    CMD_PREPARE_ERR_CODE_DEV_NOT_FOUND,
    CMD_PREPARE_ERR_CODE_NOT_SUPPORT,
};

#define SYS_BT_EVENT_TYPE_CON_STATUS (('C' << 24) | ('O' << 16) | ('N' << 8) | '\0')
#define SYS_BT_EVENT_TYPE_HCI_STATUS (('H' << 24) | ('C' << 16) | ('I' << 8) | '\0')



#define    REMOTE_DEFAULT    0x00
#define    REMOTE_SINK       0x01
#define    REMOTE_SOURCE     0x02


#define    SPP_CH       0x01
#define    HFP_CH       0x02
#define    A2DP_CH      0x04    //media
#define    AVCTP_CH     0x08
#define    HID_CH       0x10
#define    AVDTP_CH     0x20
#define    PBAP_CH      0x40
#define    HFP_AG_CH    0x80
#define    A2DP_SRC_CH  0x2000

struct sniff_ctrl_config_t {
    u16 sniff_max_interval;
    u16 sniff_mix_interval;
    u16 sniff_attemp;
    u16	sniff_timeout;
    u8	sniff_addr[6];
};

/*提供根据地址参数的命令接口
  addr指定就按指定的查找，NULL就默认正在使用那个
  cmd 用户可以使用USER_CMD_TYPE的枚举值
  param_len 传参数需要的值或者data包的长度
  param 传的是要发数据的包指针
 */
extern u32 bt_cmd_prepare_for_addr(u8 *addr, USER_CMD_TYPE cmd, u16 param_len, u8 *param);
//单个连接的时候不想管地址的命令接口
extern u32 bt_cmd_prepare(USER_CMD_TYPE cmd, u16 param_len, u8 *param);
//作为发射器时操作命令的接口
extern u32 bt_emitter_cmd_prepare(USER_CMD_TYPE cmd, u16 param_len, u8 *param);
/*根据规则生产BLE的随机地址*/
extern void bt_make_ble_address(u8 *ble_address, u8 *edr_address);


/****************蓝牙的一些状态获取接口*************************/
/*
u16 bt_get_curr_channel_state();  与  channel  判断区分
主动获取当前链路的连接状态，可以用来判断有哪些链路连接上了
*/
extern u16 bt_get_curr_channel_state();
/*
u8 bt_get_call_status(); 与BT_CALL_IND_STA 枚举的值判断
用于获取当前蓝牙电话的状态
*/
extern u8 bt_get_call_status();
/*根据地址获取电话状态*/
extern u8 bt_get_call_status_for_addr(u8 *addr);
/*根据地址获取hfp对应的音量值*/
extern u8 bt_get_call_vol_for_addr(u8 *addr);

/*当前连接的设备是不是jl测试盒*/
extern bool bt_get_remote_test_flag();
//查询当前蓝牙的状态
extern u8 bt_get_connect_status(void);
/*查询高级音频的状态*/
extern u8 bt_a2dp_get_status(void);
/*获取上电回连地址列表里面的信息*/
extern u8 bt_get_current_poweron_memory_search_index(u8 *temp_mac_addr);
/*清除上电回连地址列表计数信息*/
extern void bt_clear_current_poweron_memory_search_index(u8 inc);
/*用来获取蓝牙连接的设备个数，不包含page状态的计数*/
extern u8 bt_get_total_connect_dev(void);
/*可以通过地址查询HFP的状态*/
extern u8 bt_check_conn_is_hangup_for_addr(u8 *addr);
//bt_get_auto_connect_state有时效性，一般不用。可以用消息BT_STATUS_RECONN_OR_CONN
/*判断是否主动回连*/
extern u8 bt_get_auto_connect_state(u8 *addr);
/*根据地址获取对应设备的高级音频音量*/
extern int bt_get_music_volume(bd_addr_t addr);
/*获取原来连接设备的地址信息，只适合支持一拖一使用*/
extern u8 *bt_get_current_remote_addr(void);

enum {
    BD_ESCO_IDLE = 0,		/*当前没有设备通话中*/
    BD_ESCO_BUSY_CURRENT,	/*当前地址对应的设备通话中*/
    BD_ESCO_BUSY_OTHER,	 	/*通话中的设备非当前地址*/
};
extern u8 bt_check_esco_state_via_addr(u8 *addr);
//判断SCO/esco有没有正在使用,两个接口一样的
extern u8 bt_get_esco_coder_busy_flag();

/*sniff 的计数查询*/
extern void bt_clear_sniff_cnt(void);
extern bool bt_api_conn_mode_check(u8 enable, u8 *addr);
extern u8 bt_api_enter_sniff_status_check(u16 time_cnt, u8 *addr);
/****************蓝牙的一些状态获取接口end*************************/

/*个性化参数设置*/
/*配置测试盒测试的全局标识*/
extern void bt_set_remote_test_flag(u8 own_remote_test);
/**配置SBC的bitpool，值越大，质量好，一般用38或者53*/
extern void bt_set_sbc_cap_bitpool(u8 sbc_cap_bitpoola);
/**提供接口修改aac的bitrate，但是有些手机不一定能接受*/
extern void bt_set_aac_bitrate(u32 bitrate);
/*用户调试设置地址，6个byte*/
extern void bt_set_bt_mac_addr(u8 *addr);

/*用户调试设置name,最长32个字符*/
extern void bt_set_host_name(const char *name, u8 len);
/*用户调试设置pin code*/
extern void bt_set_pin_code(const char *code);
/*该接口用于设置上电回连需要依次搜索设备的个数。*/
extern void bt_set_auto_conn_device_num(u8 num);

/*//回连的超时设置。ms单位。但是对手机发起的连接是没作用的*/
extern void bt_set_super_timeout_value(u16 time);

/*设置电量显示发送更新的周期时间，为0表示关闭电量显示功能*/
extern void bt_set_update_battery_time(u8 time);
/*给用户设置蓝牙支持连接的个数，主要用于控制控制可发现可连接和回连流程*/
extern void bt_set_user_ctrl_conn_num(u8 num);
/*提供接口外部设置要保留hfp不要蓝牙通话*/
extern void bt_set_disable_sco_flag(bool flag);

/*提供接口外部设置简易配对参数*/
extern void bt_set_simple_pair_param(u8 io_cap, u8 oob_data, u8 mitm);
/*//回连的超时设置。ms单位。但是对手机发起的连接是没作用的*/
extern void bt_set_super_timeout_value(u16 time);
/*//回连page的超时设置。ms单位*/
extern void bt_set_page_timeout_value(u16 time);

extern void bt_set_user_background_goback(u8 en);
/*设置一个标识给库里面说明正在退出蓝牙*/
extern void bt_set_stack_exiting(u8 exit);
/*配置协议栈支持HID功能，为了兼容以前的HID独立模式，音箱SDK有使用流程*/
extern void bt_set_hid_independent_flag(bool flag);
/*配置通话使用16k的msbc还是8k的cvsd*/
extern void bt_set_support_msbc_flag(bool flag);
/*配置协议栈使用支持AAC的信息*/
extern void bt_set_support_aac_flag(bool flag);




/*有些自选接口用来实现个性化功能流程，回调函数注册，记得常来看看哟*/
//蓝牙库注册接口的函数
/*音乐的ID3信息返回接口注册函数*/
extern void bt_music_info_handle_register(void (*handler)(u8 type, u32 time, u8 *info, u16 len));
/*手机更样机音乐模式的音量同步*/
extern void bt_music_vol_change_handle_register(void (*handle)(int vol), int (*handle2)(void));
/*获取到名字后的回调函数接口注册函数*/
extern void bt_read_remote_name_handle_register(void (*remote_name)(u8 status, u8 *addr, u8 *name));
/*电量发送时获取电量等级的接口注册*/
extern void bt_get_battery_value_handle_register(int (*handle)(void));
/*支持串口功能的数据处理接口*/
extern void bt_spp_data_deal_handle_register(void (*handler)(u8 packet_type, u16 channel, u8 *packet, u16 size));
/*注册接口获取测试盒快速测试的开始状态*/
extern void bt_fast_test_handle_register(void (*handle)(void));
/*进入了DUT测试，有些状态值反馈到上层处理,参数预留以后扩展*/
extern void bt_dut_test_handle_register(void (*handle)(u8));
/*蓝牙搜索其他设备时的一些设备信息返回*/
extern void bt_inquiry_result_handle_register(u8(*handle)(char *name, u8 name_len, u8 *addr, u32 dev_class, char rssi));




/****8发射器的一些相关接口*********/
//发射器获取连接状态接口
u8 bt_emitter_get_connect_status(void);
//发射器获取当前连接链路状态接口
u16 bt_emitter_get_curr_channel_state();
//发射器获取当前音乐状态接口
u8 bt_emitter_get_a2dp_status(void);

/**发射器和接收器按键切换的时候要申请和释放资源**/
extern int bt_a2dp_source_init(void *buf, u16 len, int deal_flag);
/**发射器和接收器按键切换的时候要申请和释放资源**/
extern int bt_hfp_ag_buf_init(void *buf, int size, int deal_flag);
/*配置蓝牙协议栈处于发射器流程*/
extern void bt_emitter_set_enable_flag(u8 flag);
/*发射器启动还是暂停数据发送的接口，会发start和suspend命令*/
extern void bt_emitter_send_media_toggle(u8 *addr, u8 toggle);
/*查询当前有没有a2dp source（发射器的音频发送链路）在连接状态*/
extern u8 bt_a2dp_is_source_dev_null();

/****8发射器的一些相关接口end*********/



/*提供接口修改设备类型信息，修改什么的类型，会影响到手机显示的图标*/
extern void bt_change_hci_class_type(u32 class);
/*设备信息值举例*/
#define BD_CLASS_WEARABLE_HEADSET	0x240404/*ios10.2 display headset icon*/
#define BD_CLASS_HANDS_FREE			0x240408/*ios10.2 display bluetooth icon*/
#define BD_CLASS_MICROPHONE			0x240410
#define BD_CLASS_LOUDSPEAKER		0x240414
#define BD_CLASS_HEADPHONES			0x240418
#define BD_CLASS_CAR_AUDIO			0x240420 /*car audio类型苹果手机可以自动弹出确认框*/
#define BD_CLASS_HIFI_AUDIO			0x240428
#define BD_CLASS_PAN_DEV            0X020118
#define BD_CLASS_MOUSE              0x002580
#define BD_CLASS_KEYBOARD           0x002540
#define BD_CLASS_KEYBOARD_MOUSE     0x0025C0
#define BD_CLASS_REMOTE_CONTROL     0x00254C
#define BD_CLASS_TRANSFER_HEALTH    0x10091C




/************用户自定义HID的一些接口*******************/
typedef struct __hid_sdp_info {
    u16 vid_private;
    u16 pid_private;
    u16 ver_private;

    u8  sub_class;
    u8  country_code;
    bool virtual_cable;
    bool reconnect_initiate;
    bool sdp_disable;
    bool battery_power;
    bool remote_wake;
    bool normally_connectable;
    bool boot_device;
    u16 version;
    u16 parser_version;
    u16 profile_version;
    u16 supervision_timeout;
    u16 language;
    u16 bt_string_offset;
    u16 descriptor_len;
    u8 *descriptor;
    char *service_name;
    char *service_description;
    char *provide_name;
    void (*sdp_request_respone_callback)(u8 type);
    u8 *extra_buf;
    u8 extra_len;
} hid_sdp_info_t;

typedef struct {
    u16 chl_id;
    u16 data_len;
    u8  *data_ptr;
} hid_s_param_t;

extern u16 bt_sdp_create_diy_device_ID_service(u8 *buffer, u16 buffer_size);
extern u16 bt_sdp_create_diy_hid_service(u8 *buffer, u16 buffer_size, const u8 *hid_descriptor, u16 hid_descriptor_size);
/************用户自定义HID的一些接口 end*******************/


/*该接口会直接操作VM，可以读取蓝牙记录列表中的蓝牙地址信息
 * mac_addr：        是用来保存多个地址信息的buffer，
 * conn_device_num： 是期望读取最大个数（支持1-9）
 * id：              一般设备是0。发射器和独立的HID模式是1
*/
u8 bt_restore_remote_device_info_opt(bd_addr_t *mac_addr, u8 conn_device_num, u8 id);
/*remote dev type*/
/*0:unknow,1-android,2:apple_inc,0x03-xiaomi*/
enum {
    REMOTE_DEV_UNKNOWN  = 0,
    REMOTE_DEV_ANDROID		,
    REMOTE_DEV_IOS			,
    REMOTE_DEV_XIAOMI   	,
    REMOTE_DEV_DONGLE_SPEAK = 0xfa	,
};
/*这个接口会直接访问VM，根据地址获取已经记录好的设备厂商
 *读操作时op_flag和value参数都传0。返回值参考上面的枚举
 * */
u8 bt_remote_dev_company_ioctrl(bd_addr_t dev_addr, u8 op_flag, u8 value);


/*模式切换相关的接口*/
/*蓝牙协议栈初始化*/
int btstack_init();
/*退出蓝牙函数*/
int btstack_exit();
/*后台的恢复接口*/
void btstack_resume();//background resume
/*后台时退出蓝牙的处理接口*/
int btstack_suspend();

/*查询地址是不是杰理配套的蓝牙dongle*/
int btstack_get_dev_type_for_addr(u8 *addr);
/*查询当前有没有杰理配套的蓝牙dongle连接*/
bool bt_check_is_have_dongle_dev_conn();

/*一拖二的一些状态获取接口和操作接口*/
/*获取的值上限是bt_set_auto_conn_device_num接口配置的值,一般用于做回连策略*/
int btstack_get_num_of_remote_device_recorded();
/*根据记录的蓝牙库conn地址去操作*/
/*根据地址获取该地址对应的conn结构体地址*/
void *btstack_get_conn_device(u8 *addr);
/*获取当前有多少个已经连接的设备*/
int btstack_get_conn_devices(void *devices[], int max_num);
/*根据记录的conn结构体地址获取对应设备的地址*/
u8 *btstack_get_device_mac_addr(void *device);
/*根据记录的conn结构体地址获取对应设备的a2dp播放状态*/
int btstack_get_device_a2dp_state(void *device);
/*根据记录的conn结构体地址操作命令，命令列表也是USER_CMD_TYPE的枚举值
 * 这个接口只使用那些不需要参数的命令，需要的参数的不用这个*/
int btstack_device_control(void *device, int cmd);
/*根据记录的conn结构体地址获取对应设备的连接链路信息*/
int btstack_get_device_channel_state(void *device);
/*根据记录的conn结构体地址获取对应设备的电话状态*/
int btstack_bt_get_call_status(void *device);
/*根据记录的conn结构体地址获取对应设备的esco状态*/
int btstack_get_call_esco_status(void *device);
/*根据记录的conn结构体地址去断开对应的设备*/
void btstack_device_detach(void *device);
//记录是自动断开A2DP的地址，回连检查的时候区分手机断开的情况
void bt_stack_save_a2dp_auto_discon_addr(u8 *addr, u8 pause_music_flag);
//检查是自动断开的再回连
bool bt_stack_check_a2dp_auto_discon_addr(u8 *addr);
/*获取正在处于来电状态的设备数目*/
u8 bt_stack_get_incoming_call_num();
/*根据地址获取设备是否支持来电铃声*/
u8 btstack_get_inband_ringtone_flag_for_addr(u8 *addr);
/*根据地址配置ESCO的状态*/
void bt_api_esco_status(u8 *addr, u8 status);
/*可以获取传入地址之外的另一个设备地址信息,没有返回NULL*/
u8 *btstack_get_other_dev_addr(u8 *addr);
/*上电会根据配置读取连接过的设备地址,保持在一个数组
 * 这个接口根据数组位置获取地址出来,返回真表示获取成功，addr是获取到的地址值*/
bool btstack_get_remote_addr(u8 *addr, u8 index);
#endif
