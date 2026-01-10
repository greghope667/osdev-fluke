#include "apic.h"

#include "klib.h"
#include "mem/memory.h"
#include "mmu.h"
#include "time.h"

static const physical_t APIC_ADDRESS_P = { 0xfee0'0000 };
static const usize APIC_ADDRESS = 0xffff'ffff'fee0'0000;

static const u8 VECTOR_SPURIOUS = 255;
static const u8 VECTOR_TIMER = 254;

static u32 apic_frequency = 1'000'000'000; // Initial guess

enum apic_register {
    APIC_ID = 0x20,
    APIC_VERSION = 0x30,
    APIC_EOI = 0xb0,
    APIC_SPURIOUS_INTERRUPTS = 0xf0,
    APIC_LVT_TIMER = 0x320,
    APIC_TIMER_INITIAL = 0x380,
    APIC_TIMER_CURRENT = 0x390,
    APIC_TIMER_DIVIDE = 0x3e0,
};

#define LVT_MASK (1u << 16)
#define LVT_TIMER_PERIODIC (1u << 17)

static inline volatile u32* apic_reg(enum apic_register reg)
{
    return (volatile u32*)(APIC_ADDRESS + reg);
}

enum timer_divisor {
    DIVISOR_2 = 0b0000,
    DIVISOR_4 = 0b0001,
    DIVISOR_8 = 0b0010,
    DIVISOR_16 = 0b0011,
    DIVISOR_32 = 0b1000,
    DIVISOR_64 = 0b1001,
    DIVISOR_128 = 0b1010,
    DIVISOR_1 = 0b1011,
};

static void
measure_frequency()
{
    klog("apic.c: measuring frequency using tsc\n");

    *apic_reg(APIC_LVT_TIMER) = LVT_MASK;

    *apic_reg(APIC_TIMER_DIVIDE) = DIVISOR_1;
    u64 timeout = nanoseconds() + 10'000'000;
    u32 start = -1;

    *apic_reg(APIC_TIMER_INITIAL) = start;
    while (nanoseconds() < timeout) {}

    u32 end = *apic_reg(APIC_TIMER_CURRENT);
    *apic_reg(APIC_TIMER_INITIAL) = 0; // Stop counter

    u32 counts_per_10ms = start - end;
    apic_frequency = counts_per_10ms * 100;
    klog("apic.c: measured %u Hz\n", apic_frequency);
}

void
x86_64_apic_initialise()
{
    INIT_ONCE

    mmu_point1(
        mmu_get_address_space(),
        APIC_ADDRESS, APIC_ADDRESS_P,
        MMU_MODE_WRITE, MMU_CACHE_NONE
    );

    klog(
        "x86_64_apic_initialise: apic id %x version %x\n",
        *apic_reg(APIC_ID), *apic_reg(APIC_VERSION)
    );

    measure_frequency();
}

void
x86_64_apic_send_eoi()
{
    *apic_reg(APIC_EOI) = 0;
}

void
x86_64_apic_set_tickrate(int hz)
{
    if (hz > 0) {
        *apic_reg(APIC_TIMER_DIVIDE) = DIVISOR_1;
        *apic_reg(APIC_LVT_TIMER) = LVT_TIMER_PERIODIC | VECTOR_TIMER;
        *apic_reg(APIC_TIMER_INITIAL) = apic_frequency / hz;
    } else {
        *apic_reg(APIC_LVT_TIMER) = LVT_MASK | VECTOR_TIMER;
        *apic_reg(APIC_TIMER_INITIAL) = 0;
    }

    // Enable apic
    *apic_reg(APIC_SPURIOUS_INTERRUPTS) = (1 << 8) | VECTOR_SPURIOUS;
}
