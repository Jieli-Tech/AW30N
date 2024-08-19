#include "sd.h"
#include "gpio.h"
#include "clock.h"

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[sd]"
#include "log.h"

extern void delay(u32 i);


static void sd_gpio_all_init(const struct sdmmc_platform_data *data)
{
    for (u8 i = 0; i < 3; i ++) {
        gpio_set_mode(IO_PORT_SPILT(data->port[i]), PORT_INPUT_PULLUP_10K);
        gpio_set_mode(IO_PORT_SPILT(data->port[i]), PORT_OUTPUT_HIGH);
        gpio_set_drive_strength(IO_PORT_SPILT(data->port[i]), PORT_DRIVE_STRENGT_2p4mA);
    }
}

static void sd_gpio_iomap_init(const struct sdmmc_platform_data *data)
{
    //CMD
    gpio_set_function(IO_PORT_SPILT(data->port[0]), PORT_FUNC_SD0_CMD);
    //CLK
    gpio_set_function(IO_PORT_SPILT(data->port[1]), PORT_FUNC_SD0_CLK);
    //DA0
    gpio_set_function(IO_PORT_SPILT(data->port[2]), PORT_FUNC_SD0_DA0);
}

static void sd_gpio_init_0(const struct sdmmc_platform_data *data)
{
    sd_gpio_all_init(data);
    sd_gpio_iomap_init(data);
}

static void sd_gpio_uninit_3(const struct sdmmc_platform_data *data)
{
    gpio_disable_function(IO_PORT_SPILT(data->port[0]), PORT_FUNC_SD0_CMD);
    gpio_deinit(IO_PORT_SPILT(data->port[0]));
    gpio_disable_function(IO_PORT_SPILT(data->port[1]), PORT_FUNC_SD0_CLK);
    gpio_deinit(IO_PORT_SPILT(data->port[1]));
    gpio_disable_function(IO_PORT_SPILT(data->port[2]), PORT_FUNC_SD0_DA0);
    gpio_deinit(IO_PORT_SPILT(data->port[2]));
    for (u8 i = 0; i < 3; i ++) {
        gpio_set_mode(IO_PORT_SPILT(data->port[i]), PORT_HIGHZ);
    }
}

static void sd_gpio_suspend_4(const struct sdmmc_platform_data *data)
{
    gpio_disable_function(IO_PORT_SPILT(data->port[0]), PORT_FUNC_SD0_CMD);
    gpio_deinit(IO_PORT_SPILT(data->port[0]));
    gpio_disable_function(IO_PORT_SPILT(data->port[1]), PORT_FUNC_SD0_CLK);
    gpio_deinit(IO_PORT_SPILT(data->port[1]));
    gpio_disable_function(IO_PORT_SPILT(data->port[2]), PORT_FUNC_SD0_DA0);
    gpio_deinit(IO_PORT_SPILT(data->port[2]));
}

static void sd_gpio_resume_5(const struct sdmmc_platform_data *data)
{
    sd_gpio_all_init(data);
    for (u32 i = 0; i < 50; i ++) {
        gpio_write(data->port[1], 0);
        gpio_write(data->port[1], 1);
    }
    sd_gpio_iomap_init(data);
}

void sdmmc_0_port_init(const struct sdmmc_platform_data *data, int mode)
{
    if (mode == 0) {
        sd_gpio_init_0(data);
    } else if (mode == 3)	{
        sd_gpio_uninit_3(data);
    } else if (mode == 4)	{
        sd_gpio_suspend_4(data);
    } else if (mode == 5)	{
        sd_gpio_resume_5(data);
    }
}
int sdmmc_0_clk_detect(const struct sdmmc_platform_data *data)
{
    int val = 0;
    u8 res = 0;
    u32 *sd_clk_outreg = gpio2crossbar_outreg(data->port[1]);
    u32 data_dir_ctl = (*sd_clk_outreg) & 0b11;
    if (data_dir_ctl) {
        (*sd_clk_outreg) &= ~(0b11);
        res = 1;
    }
    if (data->detect_io_level) {
        gpio_set_mode(IO_PORT_SPILT(data->port[1]), PORT_INPUT_PULLDOWN_10K);
        delay(100);
        val = gpio_read(data->port[1]);
        val = !val;
        /* gpio_set_pull_down(data->port[1], 0); */
    } else {
        gpio_set_mode(IO_PORT_SPILT(data->port[1]), PORT_INPUT_PULLUP_10K);
        delay(100);
        val = gpio_read(data->port[1]);
        /* gpio_set_pull_up(data->port[1], 0); */
    }
    if (res) {
        (*sd_clk_outreg) |= data_dir_ctl;
        if (((*sd_clk_outreg) >> 2) == (FO_SD0_CLK >> 2)) {
            gpio_set_mode(IO_PORT_SPILT(data->port[1]), PORT_OUTPUT_LOW);
        }
    }
    return val;
}

int sdmmc_0_io_detect(const struct sdmmc_platform_data *data)
{
    int val = 0;
    u8 res = 0;
    u32 *det_io_outreg = gpio2crossbar_outreg(data->detect_io);
    u32 data_dir_ctl = (*det_io_outreg) & 0b11;
    if (data_dir_ctl) {
        (*det_io_outreg) &= ~(0b11);
        res = 1;
    }
    if (data->detect_io_level) {
        gpio_set_mode(IO_PORT_SPILT(data->detect_io), PORT_INPUT_PULLDOWN_10K);
        delay(100);
        val = gpio_read(data->detect_io);
        /* gpio_set_pull_down(data->detect_io, 0); */
    } else {
        gpio_set_mode(IO_PORT_SPILT(data->detect_io), PORT_INPUT_PULLUP_10K);
        delay(100);
        val = gpio_read(data->detect_io);
        val = !val;
        /* gpio_set_pull_up(data->detect_io, 0); */
    }
    gpio_set_mode(IO_PORT_SPILT(data->detect_io), PORT_HIGHZ);

    if (res) {
        (*det_io_outreg) |= data_dir_ctl;
    }
    return val;
}

int sdmmc_0_cmd_detect(const struct sdmmc_platform_data *data)
{
    return CMD_DECT_WITHOUT_TRANS;
}

static void sdpg_config(int enable)
{
}

int set_sd_power(int en)
{
    static u8 power_old = 0xff;
    if (power_old != en) {
        power_old = en;
        sdpg_config(en);
    }
    return 0;
}
