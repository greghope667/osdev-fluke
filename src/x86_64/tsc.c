#include "time.h"

#include "cpuid.h"
#include "io_port.h"
#include "klib.h"

u64 rdtsc();
u64 nanoseconds();
struct Time timestamp();
struct Timer_calibration tsc_calibration;

static u64
read_tsc_cpuid()
{
    if ((cpuid(0x8000'0007, 0).edx & 0x100) == 0) {
        klog("read_tsc_cpuid: TscInvariant bit not set\n");
        return 0;
    }

    auto r = cpuid(0x15, 0);
    if (r.eax == 0 || r.ebx == 0 || r.ecx == 0) {
        return 0;
    }

    return ((u64)r.ecx * r.ebx) / r.eax;
}

static u64
time_tsc_with_pit_10ms()
{
    // Disable speaker output
    io_outb(0x61, io_inb(0x61) & 0xc);

    // PIT Access channel 2, multibyte, mode0
    io_outb(0x43, 0b10110000);

    // PIT Timer = 10ms = 11931 ticks = 0x2e9b
    io_outb(0x42, 11931 & 0xff);
    io_outb(0x42, 11931 >> 8);

    // Start timer, 0x61 bit0 = timer2 enable
    io_outb(0x61, io_inb(0x61) | 1);

    u64 begin = rdtsc();

    // Wait for low output, 0x61 bit5 = output
    while (io_inb(0x61) & 0x20);

    // Wait for high output, 0x61 bit5 = output
    while (!(io_inb(0x61) & 0x20));

    u64 end = rdtsc();

    // Disable speaker output
    io_outb(0x61, io_inb(0x61) & 0xc);

    return end - begin;
}

/* Look, GCC... I know you can emit 'div' instructions. I've seen
 * you do it plenty. Why can't you just emit one here? No, I'd rather
 * not deal with your weird functions. I'm sure you worked very
 * hard on your __udivti3 and I'm sure it's wonderful, but it's
 * really not what I'm after. *Sigh*... Fine, I'll do it myself
 *
 * (Likely, GCC cannot prove we don't get a #DE exception so is
 *  trying to play it safe. I'm sure there's some way round this
 *  better than the hack that is this function)
 */
static inline struct Udiv_result { u64 quotient, remainder; }
udiv(u64 upper, u64 lower, u64 divisor)
{
    asm ("div %2" : "+d"(upper), "+a"(lower) : "r"(divisor));
    return (struct Udiv_result){ lower, upper };
}

void
tsc_init()
{
    INIT_ONCE;
    u64 freq = read_tsc_cpuid();
    if (freq) {
        klog("tsc_init: %zu Hz from cpuid.15h\n", freq);
    } else {
        klog("tsc_init: timing TSC with PIT counter...\n");
        freq = time_tsc_with_pit_10ms() * 100;
        klog("tsc_init: %zu Hz from PIT measurement\n", freq);
    }

    auto d1 = udiv(0, 1'000'000'000, freq);
    u64 period_h = d1.quotient;
    auto d2 = udiv(d1.remainder, 0, freq);
    u64 period_l = d2.quotient;

    tsc_calibration = (struct Timer_calibration){
        .frequency = freq,
        .offset = rdtsc(),
        .period_high = period_h,
        .period_low = period_l,
    };
}
