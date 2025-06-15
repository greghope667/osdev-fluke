#pragma once

#include "kdef.h"

inline u64
rdmsr(u32 reg)
{
    u32 low, high;
    asm volatile("rdmsr" : "=d"(high), "=a"(low) : "c"(reg));
    return ((u64)high << 32) | low;
}

inline void
wrmsr(u32 reg, u64 value)
{
    u32 low = value;
    u32 high = value >> 32;
    asm volatile("wrmsr" : : "d"(high), "a"(low), "c"(reg));
}

#define MSR_APIC_BASE                   0x1b
#define MSR_EXTENDED_FEATURE_ENABLES    0xc000'0080
#define MSR_STAR                        0xc000'0081
#define MSR_LSTAR                       0xc000'0082
#define MSR_CSTAR                       0xc000'0083
#define MSR_SFMASK                      0xc000'0084
#define MSR_FS_BASE                     0xc000'0100
#define MSR_GS_BASE                     0xc000'0101
#define MSR_KERNEL_GS_BASE              0xc000'0102
