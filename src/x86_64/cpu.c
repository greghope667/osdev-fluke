#include "cpu.h"
#include "mem/alloc.h"
#include "msr.h"
#include "klib.h"
#include "offsets.h"
#include "user/process.h"
#include "mmu.h"

_Static_assert(offsetof(struct Cpu, kernel_stack) == CPU_OFFSET_KERNEL_SP);
_Static_assert(offsetof(struct Cpu, user_stack) == CPU_OFFSET_USER_SP);

struct Cpu* (*cpu_array)[];
int cpu_count;

static void
push_cpu(struct Cpu* cpu)
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
    struct Cpu* cpu = xmalloc(sizeof(*cpu));
    *cpu = (struct Cpu) {
        .self = cpu,
        .kernel_stack = kernel_stack,
        .lapic_id = lapic_id,
    };
    wrmsr(MSR_GS_BASE, (usize)cpu);

    push_cpu(cpu);
}

void
cpu_context_initialise_user(struct Context* context, usize code, usize stack)
{
    *context = (struct Context) {
        .cs = GDT_USER64_CODE,
        .ss = GDT_USER_DATA,
        .rflags = 0x3202, // IOPL=3, IF set
        .rip = code,
        .rsp = stack,
    };
}

struct Process*
cpu_context_save()
{
    struct Process* process = this_cpu->process;
    struct Context* context = this_cpu->user_context;

    assert(context->cs == GDT_USER64_CODE);
    assert(process->state == RUNNING);

    process->saved_context = *context;
    process->state = FLOATING;

    this_cpu->process = nullptr;
    return process;
}

extern void exit_kernel_asm(struct Context* context) __attribute__((noreturn));

void
cpu_context_restore_and_exit(struct Process* process)
{
    assert(!this_cpu->process);
    assert(process->state == FLOATING);
    assert(process->saved_context.cs == GDT_USER64_CODE);

    wrmsr(MSR_KERNEL_GS_BASE, 0);
    wrmsr(MSR_FS_BASE, 0);
    mmu_set_address_space(process->vm.page_map);

    this_cpu->process = process;
    process->state = RUNNING;
    exit_kernel_asm(&process->saved_context);
}

static void idle()
{
    for (;;)
        asm("hlt");
}

void
cpu_exit_idle()
{
    assert(!this_cpu->process);
    struct Context context = {
        .cs = GDT_KERNEL_CODE,
        .ss = GDT_KERNEL_DATA,
        .rflags = 0x202, // IF set
        .rip = (usize)&idle,
        .rsp = this_cpu->kernel_stack,
    };
    exit_kernel_asm(&context);
}
