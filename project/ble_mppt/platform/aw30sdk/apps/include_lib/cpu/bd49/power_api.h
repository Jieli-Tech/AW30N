#ifndef POWER_API_H
#define POWER_API_H

void sys_power_down(u32 usec);

void sys_softoff();

void lowpower_mode(u8 mode);

#endif
