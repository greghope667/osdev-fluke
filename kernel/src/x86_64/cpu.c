#include "cpu.h"
#include "msr.h"
#include "klib.h"
#include "offsets.h"
#include "mmu.h"
#include "tls.h"

void
cpu_context_initialise_user(struct Registers* context, usize code, usize stack)
{
    // TODO: push 0 to align stack/add return address?
    *context = (struct Registers) {
        .cs = GDT_USER64_CODE,
        .ss = GDT_USER_DATA,
        .rflags = 0x3202, // IOPL=3, IF set
        .rip = code,
        .rsp = stack,
    };
}

struct Thread_context*
cpu_context_save()
{
    auto thread = this_tls->current_thread;
    auto context = this_tls->user_context;

    assert(context->cs == GDT_USER64_CODE);
    assert(thread->state == RUNNING);

    thread->ctx = *context;
    thread->state = FLOATING;

    this_tls->current_thread = nullptr;
    return thread;
}

extern void exit_kernel_asm(struct Registers* context) __attribute__((noreturn));

void
cpu_context_restore_and_exit(struct Thread_context* thread)
{
    assert(!this_tls->current_thread);
    assert(thread->state == FLOATING);
    assert(thread->ctx.cs == GDT_USER64_CODE);

    wrmsr(MSR_KERNEL_GS_BASE, 0);
    wrmsr(MSR_FS_BASE, 0);
    mmu_set_address_space((struct Page_map){ thread->page_map_top });

    this_tls->current_thread = thread;
    thread->state = RUNNING;
    exit_kernel_asm(&thread->ctx);
}

static void idle()
{
    for (;;)
        asm("hlt");
}

void
cpu_exit_idle()
{
    assert(!this_tls->current_thread);
    struct Registers context = {
        .cs = GDT_KERNEL_CODE,
        .ss = GDT_KERNEL_DATA,
        .rflags = 0x202, // IF set
        .rip = (usize)&idle,
        .rsp = this_tls->kernel_stack,
    };
    exit_kernel_asm(&context);
}
