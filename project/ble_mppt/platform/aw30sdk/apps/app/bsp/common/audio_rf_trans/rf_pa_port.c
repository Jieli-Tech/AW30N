#include "app_config.h"
#include "rf_pa_port.h"
#include "gpio.h"

#define LOG_TAG_CONST       NORM
#define LOG_TAG             "[rf_radio]"
#include "log.h"


#if RF_PA_EN
void rf_pa_io_sel(void)
{
    log_info("%s() %d\n", __func__, __LINE__);
#if RF_PA_POWER_SUPPLY
    /* 如果PA使用电池供电，则无需设置； */
    /* 若需要使用芯片LDO进行供电，则需要打开该配置(RF_PA_POWER_SUPPLY)并设置对应的芯片LDO供电IO(即RF_PA_POWER_SUPPLY_IO)	 */
    gpio_set_mode(IO_PORT_SPILT(RF_PA_POWER_SUPPLY_IO), PORT_OUTPUT_HIGH);
#endif
    SFR(JL_IOMC->OCH_CON0, 12, 5, 16);
    SFR(JL_IOMC->OCH_CON0, 18, 5, 17);
    gpio_set_fun_output_port(RF_PA_POWER_TX_IO, FO_GP_OCH2, 1, 1);
    gpio_set_fun_output_port(RF_PA_POWER_RX_IO, FO_GP_OCH3, 1, 1);
    gpio_hw_direction_output(IO_PORT_SPILT(RF_PA_POWER_TX_IO), 0);
    gpio_hw_direction_output(IO_PORT_SPILT(RF_PA_POWER_RX_IO), 0);
}
#endif
