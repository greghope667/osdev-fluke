#include "tls.h"
#include "klib.h"
#include "offsets.h"
#include "mem/alloc.h"
#include "msr.h"

_Static_assert(offsetof(struct Tls, kernel_stack) == CPU_OFFSET_KERNEL_SP);
_Static_assert(offsetof(struct Tls, user_stack) == CPU_OFFSET_USER_SP);

struct Tls* (*cpu_array)[];
int cpu_count;

static void
push_cpu(struct Tls* cpu)
{
    static int length = 0;

    if (cpu_count == length) {
        int new_length = MAX(length * 2, 4);

        int old_size = length * 8;
        int new_size = new_length * 8;

        if (new_size > ALLOC_MAX)
            panic("too many cpus");

        void* new_cpu_array = xmalloc(new_size);
        if (length > 0) {
            memcpy(new_cpu_array, cpu_array, old_size);
            kfree(cpu_array, old_size);
        }

        cpu_array = new_cpu_array;
        length = new_length;
    }

    (*cpu_array)[cpu_count++] = cpu;
}

void
x86_64_cpu_create_tls(u8 lapic_id, usize kernel_stack)
{
    struct Tls* tls = xmalloc(sizeof(*tls));
    *tls = (struct Tls) {
        .self = tls,
        .kernel_stack = kernel_stack,
        .lapic_id = lapic_id,
    };
    wrmsr(MSR_GS_BASE, (usize)tls);

    push_cpu(tls);
}

struct Tls*
get_tls()
{
    return this_tls->self;
}
