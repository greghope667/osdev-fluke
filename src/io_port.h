#pragma once

#include "types.h"

inline void
io_outb(u16 addr, u8 val)
{
    asm volatile ("outb\t%1, %0" : : "Nd"(addr), "a"(val));
}

inline u8
io_inb(u16 addr)
{
    u8 val;
    asm volatile ("inb\t%1, %0" : "=a"(val) : "Nd"(addr));
    return val;
}
