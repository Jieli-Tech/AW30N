
#include "app_modules.h"
#include "cpu.h"
#include "testbox_uart_update.h"
#include "uart_v2.h"
#include "msg/msg.h"
#include "code_v2/update_loader_download.h"
#include "sys_timer.h"
#include "code_v2/update.h"
#include "code_v2/update_loader_download.h"


#define LOG_TAG_CONST       UPDATE
/* #define LOG_TAG_CONST       OFF */
#define LOG_TAG             "[update]"
#include "log.h"

#if TESTBOX_UART_UPDATE_EN

static u8 tmp_buf[DMA_BUF_LEN] sec(.uart_update_buf1) __attribute__((aligned(4))) = {0};
static u8 uart_buf[DMA_BUF_LEN] sec(.uart_update_buf1) __attribute__((aligned(4)));

static volatile u8 is_testbox_uart_active = 0;	//串口升级空闲

#define RETRY_TIME          4
#define PROTOCAL_SIZE       528
#define SYNC_SIZE           6
#define SYNC_MARK0          0xAA
#define SYNC_MARK1          0x55

#define CMD_UPDATE_START    0x01
#define CMD_UPDATE_READ     0x02
#define CMD_UPDATE_END      0x03
#define CMD_SEND_UPDATE_LEN 0x04
#define CMD_KEEP_ALIVE      0x05

#define SEEK_SET	0	/* Seek from beginning of file.  */
#define SEEK_CUR	1	/* Seek from current position.  */
#define SEEK_END	2	/* Seek from end of file.  */

#define TESTBOX_UART_UPDATE_FLAG      0xAB539610
#define SDK_JUMP_FLAG                 "SDKJUMP"
typedef struct testbox_uart_update_info {
    u32 baud;
    u32 uart_update_flag;
    u8  io_port[6];
} TESTBOX_UP_UART;

struct uart_upgrade_cmd {
    u8 cmd;
    u32 addr;
    u32 lenOfloader;
    u16 crcOfloader;
    u16 crc_self;
    u8 opt;//bit0 压缩，bit1加密
    u8 baud;//baud * 10K
    u8 res;
} _GNU_PACKED_;

typedef union {
    u8 raw_data[PROTOCAL_SIZE + SYNC_SIZE];
    struct {
        u8 mark0;
        u8 mark1;
        u16 length;
        u8 data[PROTOCAL_SIZE + 2]; //最后CRC16
    } data;
} protocal_frame_t;

struct file_info {
    u8 cmd;
    u32 addr;
    u32 len;
} __attribute__((packed));

static u8 packet_flag = 0; // 标识是否串口是否接收到数据包
static protocal_frame_t *protocal_frame;
static struct uart_upgrade_cmd ut_cmd;
static u8 uart_step = UART_UPDATE_START; // 升级状态机
static u32 loader_len = 0;  // 记录接收的loader长度
static volatile u32 file_offset = 0;
static const u8 ack_cmd[5] = {0x55, 0xaa, 1, 0x20, 0x22} ;
static const u8 ack_cmd_1[6] = {0x55, 0xaa, 2, 0x20, 0xAA, 0xAA} ;
static u32 update_baudrate = 0;

extern u16 CRC16(const void *ptr, u32 len);
extern void uart_update_check(void);

extern const char updata_file_name[];

u8 uart_update_get_step(void)
{
    return uart_step;
}

static void uart_update_set_step(u8 step)
{
    uart_step = step;
}

static void uart_update_cmd_ack(u8 flag)
{
    u32 tx_buf[2];
    if (flag == 1) {
        memcpy(tx_buf, ack_cmd_1, sizeof(ack_cmd_1));
        uart_send_bytes(UART_UPDATE_NUM, (void *)tx_buf, sizeof(ack_cmd_1));
    } else {
        memcpy(tx_buf, ack_cmd, sizeof(ack_cmd));
        uart_send_bytes(UART_UPDATE_NUM, (void *)tx_buf, sizeof(ack_cmd));
    }
}

void uart_update_maskrom_start(void *buf, u32 len)
{
    if (uart_update_get_step() == UART_UPDATE_START) {
        memcpy((u8 *)&ut_cmd, (u8 *)buf, sizeof(ut_cmd));
        uart_update_cmd_ack(ut_cmd.res);
        uart_wait_tx_idle(UART_UPDATE_NUM, 200);
        loader_len = 0;
        if (ut_cmd.baud) {
            uart_set_baudrate(UART_UPDATE_NUM, ut_cmd.baud * 10000);
        } else {
            uart_set_baudrate(UART_UPDATE_NUM, 10 * 10000);
        }
        if (ut_cmd.res == 1) {
            uart_update_set_step(UART_UPDATE_OTA);
            post_event(EVENT_TEXTBOX_UART_UPDATE);
        } else { // 按照原来的流程
            uart_update_set_step(UART_UPDATE_OTA_RECV);
        }
    }
}

static bool uart_send_and_recv_packet(u8 *buf, u16 length, u32 timeout)
{
    bool ret;
    u16 crc;
    u8 *buffer;

    buffer = (u8 *)protocal_frame;
    protocal_frame->data.mark0 = SYNC_MARK0;
    protocal_frame->data.mark1 = SYNC_MARK1;
    protocal_frame->data.length = length;
    memcpy((char *)&buffer[4], buf, length);
    crc = CRC16(buffer, length + SYNC_SIZE - 2);
    memcpy(buffer + 4 + length, &crc, 2);

    packet_flag = 0;  // 清零是否接收到数据包标志
    uart_send_bytes(UART_UPDATE_NUM, (u8 *)protocal_frame, length + SYNC_SIZE);
    uart_wait_tx_idle(UART_UPDATE_NUM, 200);

    timeout = ut_msecs_to_jiffies(timeout);
    u32 _timeout = timeout + ut_get_jiffies();
    while (!packet_flag) {
        if ((timeout != 0) && time_before(_timeout, ut_get_jiffies())) {
            return FALSE;
        }
        asm volatile("nop");
        wdt_clear();
    }

    return TRUE;
}

static void uart_update_recv(u8 cmd, u8 *buf, u32 len)
{
    switch (cmd) {
    case CMD_UPDATE_START:
        memcpy(&update_baudrate, buf, 4);
        uart_set_baudrate(UART_UPDATE_NUM, update_baudrate);
        break;

    case CMD_UPDATE_END:
        break;

    case CMD_SEND_UPDATE_LEN:
        break;

    default:
        break;
    }
}

static bool uart_update_cmd(u8 cmd, u8 *buf, u32 len)
{
    u8 *pbuf, i;
    for (i = 0; i < RETRY_TIME; i++) {
        pbuf = protocal_frame->data.data;
        pbuf[0] = cmd;
        memcpy(pbuf + 1, buf, len);
        /* packet_flag = 0; */
        if (uart_send_and_recv_packet(pbuf, len + 1, 350) == FALSE) {
            continue;
        }
        if (cmd == protocal_frame->data.data[0]) {
            uart_update_recv(cmd, &protocal_frame->data.data[1], protocal_frame->data.length - 1);
            return TRUE;
        }
    }
    putchar('F');
    return FALSE;
}

static void uart_data_decode(u8 *buf, u16 len)
{
    static volatile u16 rx_cnt;  //收数据计数
    u16 crc, crc0, i, ch;
    /* printf("decode_len:%d\n", len); */
    /* put_buf(buf, len); */
    for (i = 0; i < len; i++) {
        ch = buf[i];
__recheck:
        if (rx_cnt == 0) {
            if (ch == SYNC_MARK0) {
                protocal_frame->raw_data[rx_cnt++] = ch;
            }
        } else if (rx_cnt == 1) {
            protocal_frame->raw_data[rx_cnt++] = ch;
            if (ch != SYNC_MARK1) {
                rx_cnt = 0;
                goto __recheck;
            }
        } else if (rx_cnt < 4) {
            protocal_frame->raw_data[rx_cnt++] = ch;
        } else {
            protocal_frame->raw_data[rx_cnt++] = ch;
            if (rx_cnt == (protocal_frame->data.length + SYNC_SIZE)) {
                rx_cnt = 0;
                crc = CRC16(protocal_frame->raw_data, protocal_frame->data.length + SYNC_SIZE - 2);
                memcpy(&crc0, &protocal_frame->raw_data[protocal_frame->data.length + SYNC_SIZE - 2], 2);
                if (crc0 == crc) {
                    packet_flag = 1;
                }
            }
        }
    }
}

static u32 uart_dev_receive_data(void *buf, u32 relen, u32 addr)
{
    u8 i;
    struct file_info file_cmd;
    for (i = 0; i < RETRY_TIME; i++) {
        if (i > 0) {
            putchar('r');
        }
        file_cmd.cmd = CMD_UPDATE_READ;
        file_cmd.addr = addr;
        file_cmd.len = relen;
        if (uart_send_and_recv_packet((u8 *)&file_cmd, sizeof(file_cmd), 350) == FALSE) {
            continue;
        }
        memcpy(&file_cmd, protocal_frame->data.data, sizeof(file_cmd));
        if ((file_cmd.cmd != CMD_UPDATE_READ) || (file_cmd.addr != addr) || (file_cmd.len != relen)) {
            continue;
        }
        memcpy(buf, &protocal_frame->data.data[sizeof(file_cmd)], protocal_frame->data.length - sizeof(file_cmd));
        return (protocal_frame->data.length - sizeof(file_cmd));
    }
    putchar('R');
    return -1;
}

static u16 uart_f_read(void *fp, u8 *buf, u16 relen)
{
    u32 len;
    wdt_clear();
    len = uart_dev_receive_data(buf, relen, file_offset);
    if (len == (u32) - 1) {
        return (u16) - 1;
    }
    file_offset += len;
    return len;
}

static int uart_f_seek(void *fp, u8 type, u32 offset)
{
    if (type == SEEK_SET) {
        file_offset = offset;
    } else if (type == SEEK_CUR) {
        file_offset += offset;
    }
    return 0;//FR_OK;
}

static u16 uart_f_open(void)
{
    return 1;
}

static u16 uart_f_stop(u8 err)
{
    return uart_update_cmd(CMD_UPDATE_END, &err, 1);
}

static const update_op_api_t uart_dev_update_op = {
    .ch_init = NULL,
    .f_open = uart_f_open,
    .f_read = uart_f_read,
    .f_seek = uart_f_seek,
    .f_stop = uart_f_stop,
    .notify_update_content_size = NULL,
    .ch_exit = NULL,
};

static void uart_update_ota_start(void)
{
    uart_update_cmd(CMD_UPDATE_START, NULL, 0);
}

static void app_testbox_loader_ufw_update_private_param_fill(UPDATA_PARM *p)
{
    log_error("app_testbox_loader_ufw_update_private_param_fill\n");
    /* u16 up_type = p->parm_type; */
    struct testbox_uart_update_info testbox_uart_update_parm;
    testbox_uart_update_parm.baud = update_baudrate;
    testbox_uart_update_parm.uart_update_flag = TESTBOX_UART_UPDATE_FLAG;

    char io_port_stirng[6] = {0};
    if (TCFG_UART_UPDATE_PORT >= IO_PORT_DP) {
        if (TCFG_UART_UPDATE_PORT == IO_PORT_DP) {
            memcpy(io_port_stirng, "USBDP", 5);
        }
        if (TCFG_UART_UPDATE_PORT == IO_PORT_DM) {
            memcpy(io_port_stirng, "USBDM", 5);
        }
    } else {
        strcat((char *)io_port_stirng, "P");
        u8 target_string = 0;
        target_string = TCFG_UART_UPDATE_PORT / 16 + 'A';
        u8 string_len = strlen(io_port_stirng);
        io_port_stirng[string_len] = (char)target_string;   //PA

        string_len = strlen(io_port_stirng);
        target_string = (TCFG_UART_UPDATE_PORT / 16 % 10) + '0';    //PA0
        io_port_stirng[string_len] = (char)target_string;
        target_string = (TCFG_UART_UPDATE_PORT / 16 / 10) + '0';    //PA00
        io_port_stirng[string_len + 1] = (char)target_string;
    }
    memcpy(&testbox_uart_update_parm.io_port[0], &io_port_stirng[0], 6);

    ASSERT(sizeof(struct testbox_uart_update_info) <= sizeof(p->parm_priv), "uart update parm size limit");

    memcpy(p->parm_priv, &testbox_uart_update_parm, sizeof(testbox_uart_update_parm));

    memcpy(p->file_patch, updata_file_name, strlen(updata_file_name));

#if UPDATA_KEEP_IO_ENABLE
    update_keep_io(p);
#endif
}

static void testbox_uart_update_param_private_handle(UPDATA_PARM *p)
{
    u16 up_type = p->parm_type;
    struct testbox_uart_update_info testbox_uart_update_parm;
    testbox_uart_update_parm.baud = update_baudrate;
    testbox_uart_update_parm.uart_update_flag = TESTBOX_UART_UPDATE_FLAG;

    ASSERT(sizeof(struct testbox_uart_update_info) <= sizeof(p->parm_priv), "uart update parm size limit");

    memcpy(p->parm_priv, &testbox_uart_update_parm, sizeof(testbox_uart_update_parm));

    memcpy(p->file_patch, updata_file_name, strlen(updata_file_name));
}

void testbox_uart_update_jump_flag_fill(void)
{
    u8 *p = (u8 *)BOOT_STATUS_ADDR;
    memcpy(p, SDK_JUMP_FLAG, sizeof(SDK_JUMP_FLAG));
}

static void testbox_uart_update_before_jump_handle(int up_type)
{
    testbox_uart_update_jump_flag_fill();
    log_info(">>>[test]:latch reset update\n");
    common_update_before_jump_reset_handle();
}

static void testbox_uart_update_state_cbk(int type, u32 state, void *priv)
{
    update_ret_code_t *ret_code = (update_ret_code_t *)priv;

    switch (state) {
    case UPDATE_CH_EXIT:
        if ((0 == ret_code->stu) && (0 == ret_code->err_code)) {
            update_mode_api_v2(type,
                               app_testbox_loader_ufw_update_private_param_fill,
                               testbox_uart_update_before_jump_handle);
        } else {
#if UPDATA_KEEP_IO_ENABLE
            log_info("update fail, latch reset!!!\n");
            update_get_latch_romio(NULL);
            common_update_before_jump_reset_handle();
#else
            log_info("update fail, cpu reset!!!\n");
            system_reset(UPDATE_FLAG);
#endif
        }
        break;
    }
}

static void testbox_uart_update_check(void)
{
    update_mode_info_t info = {
        .type = TESTBOX_UART_UPDATA,
        .state_cbk = testbox_uart_update_state_cbk,
        .p_op_api = (update_op_api_t *) &uart_dev_update_op,
        .task_en = 0,
    };
    if (ut_cmd.res == 1) {
        info.type = TESTBOX_UART_UPDATA_2;
    }
    app_active_update_task_init(&info);
}

int uart_update_ota_loop(u8 *buf, u32 len)
{
    if (!CONFIG_UPDATE_TESTBOX_UART_EN) {
        return UPDATA_NON;
    }
    uart_update_ota_start();
    uart_update_ota_start();
    testbox_uart_update_check();
    return 1;
}

static u8 uart_update_deal(u8 *buf, u32 len)
{
    switch (uart_step) {
    case UART_UPDATE_START:
        return 0;// 需要后续处理
        break;
    case UART_UPDATE_OTA_RECV:
        loader_len += len;
        if (ut_cmd.lenOfloader <= loader_len) {
            /* log_info("recv loader ok, recv loader len %u, want len %u\n",loader_len,ut_cmd.lenOfloader); */
            uart_update_set_step(UART_UPDATE_OTA);
            post_event(EVENT_TEXTBOX_UART_UPDATE);
        }
        return 1;// 不需要后续处理
        break;
    case UART_UPDATE_OTA:
        uart_data_decode(buf, len);
        return 1;// 不需要后续处理
        break;
    }
    return 0;
}
u16 testbox_timer_id;
void testbox_uart_active_set(void *priv);
/*--------------------uart1设备模块--------------------*/
static void uart_update_isr_hook(uart_dev uart_num, enum uart_event event)
{
    /* log_info("isr hook\n"); */
    if (testbox_timer_id) {
        sys_timer_re_run(testbox_timer_id);
    } else {
        testbox_timer_id = sys_timeout_add(NULL, testbox_uart_active_set, 2000);
    }

    if (event & (UART_EVENT_RX_DATA | UART_EVENT_RX_TIMEOUT)) {
        is_testbox_uart_active = 1;
        u32 rx_len = DMA_BUF_LEN;
        u32 len = uart_recv_bytes(UART_UPDATE_NUM, protocal_frame->raw_data, rx_len);
        if (len != 0) {
            /* log_info("uart_rx_ot len : %d", len); */
            /* log_info_hexdump(tmp_buf, len); */
            if (uart_update_deal(protocal_frame->raw_data, len)) {
                return;
            }
            uart_update_data_deal(protocal_frame->raw_data, len);
        }
        /* is_testbox_uart_active = 0; */
    }

}

static int testbox_uart_dev_init()
{
    memset((void *)uart_buf, 0, DMA_BUF_LEN);
    struct uart_config uart_update_config = {
        .baud_rate = 9600,
        .tx_pin = TCFG_UART_UPDATE_PORT,
        .rx_pin = TCFG_UART_UPDATE_PORT,
        .parity = UART_PARITY_DISABLE,
    };

    int ret = uart_init(UART_UPDATE_NUM, &uart_update_config);
    if (ret < 0) {
        log_error("uart_init err 0x%x \n", ret);
        return ret;
    } else {
        log_info("uart_init succ\n");
    }


    struct uart_dma_config uart_update_dma = {
        .rx_timeout_thresh = 1070,
        .frame_size = DMA_BUF_LEN,//32,
        .event_mask = UART_EVENT_TX_DONE | UART_EVENT_RX_DATA | UART_EVENT_RX_FIFO_OVF | UART_EVENT_RX_TIMEOUT,
        .irq_priority = 3,
        .irq_callback = uart_update_isr_hook,
        .rx_cbuffer = uart_buf,
        .rx_cbuffer_size = DMA_BUF_LEN,
    };

    protocal_frame = (protocal_frame_t *)tmp_buf;

    ret = uart_dma_init(UART_UPDATE_NUM, &uart_update_dma);
    if (ret != 0) {
        log_error("uart_dma_init err 0x%x \n", ret);
        return ret;
    } else {
        log_info("uart_dma_init succ\n");
    }

    /* uart_dump(); */
    return 0;

}

/*--------------------初始化接口--------------------*/
void testbox_uart_update_init(void)
{
    if (0 != testbox_uart_dev_init()) {			//配置串口
        log_info("uart_update init fail!\n");
        return;
    }
    uart_update_data_init();
}

void testbox_uart_active_set(void *priv)
{
    is_testbox_uart_active = 0;      //35*10Ms
    testbox_timer_id = 0;
}
u8 testbox_uart_idle_query(void)
{
    return !is_testbox_uart_active;
}
REGISTER_LP_TARGET(testbox_uart_update_lp_target) = {
    .name = "testbox_uart_update",
    .is_idle = testbox_uart_idle_query,
};

#else

void testbox_uart_update_init(void)
{
    log_info("UART_UPDATE_EN is not enable \n");
}

#endif


