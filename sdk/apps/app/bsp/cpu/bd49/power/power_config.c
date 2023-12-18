#include "asm/power_interface.h"

//-----------------------------------------------------------------------------------------------------------------------
/* config
 */
#include "app_config.h"
#include "gpio.h"

#define POWER_PARAM_CONFIG				1
#define POWER_PARAM_BTOSC_HZ			24000000//TCFG_CLOCK_OSC_HZ
#define POWER_PARAM_VDDIOM_LEV			TCFG_VDDIOM_LEVEL
#define POWER_PARAM_VDDIOW_LEV			0
#define POWER_PARAM_OSC_TYPE			OSC_TYPE_LRC

//-----------------------------------------------------------------------------------------------------------------------
/* power_param
 */
struct _power_param power_param = {
    .config         = POWER_PARAM_CONFIG,
    .btosc_hz       = POWER_PARAM_BTOSC_HZ,
    .vddiom_lev     = POWER_PARAM_VDDIOM_LEV,
    .vddiow_lev     = POWER_PARAM_VDDIOW_LEV,
    .osc_type       = POWER_PARAM_OSC_TYPE,
};

//----------------------------------------------------------------------------------------------------------------------
/* power_pdata
 */
struct _power_pdata power_pdata = {
    .power_param_p  = &power_param,
};

//----------------------------------------------------------------------------------------------------------------------
void board_power_init()
{
    phw_control(PCONTROL_DCVDD_CAP_SW, 0);
    phw_control(PCONTROL_FLASH_PG_VDDIO, 0);
    phw_control(PCONTROL_PD_VDDIO_KEEP, VDDIO_KEEP_TYPE_NORMAL);
    phw_control(PCONTROL_SF_VDDIO_KEEP, VDDIO_KEEP_TYPE_NORMAL);

    power_init(&power_pdata);

//#if (!TCFG_CHARGE_ENABLE)
//    power_set_mode(TCFG_LOWPOWER_POWER_SEL);
//#endif
}
