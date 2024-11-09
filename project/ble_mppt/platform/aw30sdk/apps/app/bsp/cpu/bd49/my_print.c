#include "config.h"
/* #include "clock.h" */
#include "uart.h"
#include "app_config.h"



AT(.log_ut.text.cache.L2)
void put_u4hex(u8 dat)
{
    dat = 0xf & dat;

    if (dat > 9) {
        putchar(dat - 10 + 'A');
    } else {
        putchar(dat + '0');
    }
}
AT(.log_ut.text.cache.L2)
static void xput_u32hex(u32 dat)
{
    putchar('0');
    putchar('x');
    put_u4hex(dat >> 28);
    put_u4hex(dat >> 24);

    put_u4hex(dat >> 20);
    put_u4hex(dat >> 16);

    put_u4hex(dat >> 12);
    put_u4hex(dat >> 8);

    put_u4hex(dat >> 4);
    put_u4hex(dat);
    putchar('\n');
}
#if 0

AT(.log_ut.text.cache.L2)
void put_u32hex0(u32 dat)
{
    putchar('0');
    putchar('x');
    put_u4hex(dat >> 28);
    put_u4hex(dat >> 24);

    put_u4hex(dat >> 20);
    put_u4hex(dat >> 16);

    put_u4hex(dat >> 12);
    put_u4hex(dat >> 8);

    put_u4hex(dat >> 4);
    put_u4hex(dat);
    putchar(' ');
}
AT(.log_ut.text.cache.L2)
u32 u32d_tab[] = {
    1000000000,
    100000000,
    10000000,
    1000000,
    100000,
    10000,
    1000,
    100,
    10,
    1,
};
AT(.log_ut.text.cache.L2)
void put_u32d(u32 dat)
{
    u32 i, tmp;
    u32 zero = 0;
    putchar(' ');
    //putchar('d');
    for (i = 0; i < (sizeof(u32d_tab) / 4); i++) {
        tmp = dat / u32d_tab[i];
        //put_u32hex(dat);
        //put_u32hex(tmp);
        //putchar('\n');
        if (tmp) {
            zero++;
            dat -= tmp * u32d_tab[i];
        }
        if ((0 != tmp) || (0 != zero)) {
            put_u4hex(tmp);
        }

    }
    if (0 == zero) {
        put_u4hex(tmp);
    }
    putchar('\n');
}
void put_u16hex0(u16 dat)
{
    put_u4hex(dat >> 12);
    put_u4hex(dat >> 8);

    put_u4hex(dat >> 4);
    put_u4hex(dat);
    putchar(' ');
}
void put_u16hex(u16 dat)
{
    put_u4hex(dat >> 12);
    put_u4hex(dat >> 8);

    put_u4hex(dat >> 4);
    put_u4hex(dat);
    putchar('\n');
}
#endif



AT(.log_ut.text.cache.L2)
void put_u8hex(u8 dat)
{
    put_u4hex(dat >> 4);
    put_u4hex(dat);
    putchar(' ');
}
AT(.log_ut.text.cache.L2)
void printf_buf(const u8 *buf, int len)
{

    u32 i ;
    //putchar('\n') ;
    //xput_u32hex((u32)buf) ;
    // put_u32hex(len);

    for (i = 0 ; i < len ; i++) {
        if ((i % 16) == 0) {
            if (0 != i) {
                // put_u32hex0(len);
                // put_u32hex0(i);
                putchar('\n') ;
            }
        }

        put_u8hex(buf[i]) ;
    }
    putchar('\n') ;
}

void put_buf(u8 *buf, u32 len)
{
    return printf_buf(buf, len);
}

__attribute__((weak))void put_u32hex(u32 dat)
{
    putchar('0');
    putchar('x');

    put_u4hex(dat >> 28);
    put_u4hex(dat >> 24);
    put_u4hex(dat >> 20);
    put_u4hex(dat >> 16);
    put_u4hex(dat >> 12);
    put_u4hex(dat >> 8);
    put_u4hex(dat >> 4);
    put_u4hex(dat);
    putchar(' ');
}


