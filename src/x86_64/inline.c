#include "msr.h"
#include "io_port.h"

u64 rdmsr(enum MSR reg);
void wrmsr(enum MSR reg, u64 value);

u8 io_inb(u16 addr);
void io_outb(u16 addr, u8 byte);
