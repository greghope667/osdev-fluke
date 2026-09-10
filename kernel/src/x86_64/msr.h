#pragma once

#include "kdef.h"

enum MSR : u32 {
    MSR_APIC_BASE                       = 0x0000'001b,
    MSR_EFER                            = 0xc000'0080,
    MSR_STAR                            = 0xc000'0081,
    MSR_LSTAR                           = 0xc000'0082,
    MSR_CSTAR                           = 0xc000'0083,
    MSR_SFMASK                          = 0xc000'0084,
    MSR_FS_BASE                         = 0xc000'0100,
    MSR_GS_BASE                         = 0xc000'0101,
    MSR_KERNEL_GS_BASE                  = 0xc000'0102,
};

#define MSR_EFER_SCE (1 << 0)

inline u64
rdmsr(enum MSR reg)
{
    u32 low, high;
    asm volatile("rdmsr" : "=d"(high), "=a"(low) : "c"(reg));
    return ((u64)high << 32) | low;
}

inline void
wrmsr(enum MSR reg, u64 value)
{
    u32 low = value;
    u32 high = value >> 32;
    asm volatile("wrmsr" : : "d"(high), "a"(low), "c"(reg));
}
