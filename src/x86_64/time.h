#pragma once

#include "kdef.h"

extern struct Timer_calibration {
    /* Period of a tick, in nanoseconds, as a 64.64 fixed point number */
    u64 period_high, period_low;
    u64 frequency;
    u64 offset;
} tsc_calibration;

void tsc_init();

inline u64
rdtsc()
{
    u64 h, l;
    asm volatile ("rdtsc" : "=d"(h), "=a"(l));
    return (h << 32) | l;
}

inline u64
nanoseconds()
{
    u128 ticks = rdtsc() - tsc_calibration.offset;
    return
        (ticks * tsc_calibration.period_high) +
        ((ticks * tsc_calibration.period_low) >> 64);
}

struct Time {
    u64 seconds, nanoseconds;
};

inline struct Time
timestamp()
{
    u64 ns = nanoseconds();
    return (struct Time){ ns / 1'000'000'000, ns % 1'000'000'000 };
}
