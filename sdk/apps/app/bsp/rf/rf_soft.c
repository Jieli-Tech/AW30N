#pragma bss_seg(".rf_soft.data.bss")
#pragma data_seg(".rf_soft.data")
#pragma const_seg(".rf_soft.text.const")
#pragma code_seg(".rf_soft.text")
#pragma str_literal_override(".rf_soft.text.const")

#include "common.h"
#include "typedef.h"
#include "hwi.h"

/* #define LOG_TAG_CONST       NORM */
#define LOG_TAG_CONST       OFF
#define LOG_TAG             "[rf_soft]"
#include "log.h"



SET(interrupt(""))
void t_soft0_isr()
{
    log_info("soft0 isr");
    /* asm("trigger"); */
    u32 volatile xxxx = JL_PORTA->OUT;
    bit_clr_swi(0);
}

SET(interrupt(""))
void t_xxxx_isr()
{
    log_info("xxxx isr");
    while (1);
    /* bit_clr_swi(0); */
}

void test_soft(void)
{
    u32 *entry = (void *)IRQ_MEM_ADDR;
    for (u32 i = 0; i < 128; i++) {
        /* entry[i] = (void *)t_xxxx_isr; */
    }
    log_info("test soft");
    HWI_Install(IRQ_SOFT0_IDX, (u32)t_soft0_isr, 0) ;
    bit_set_swi(0);
}

