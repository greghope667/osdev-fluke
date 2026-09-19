#include "cpu.h"
#include "mem/alloc.h"
#include "msr.h"
#include "klib.h"
#include "offsets.h"
#include "mmu.h"
#include "tls.h"
#include "cr.h"

struct FXSave_region {
    u16 fcw;
    u16 fsw;
    u8 ftw;
    u8 _rsvd_hdr;
    u16 fop;
    u64 ip;
    u64 dp;
    u32 mxcsr;
    u32 mxcsr_mask;
    u128 st_mm_regs[8];
    u128 xmm_regs[16];
    u128 _rsvd[6];
} __attribute__((aligned(16)));

_Static_assert(sizeof(struct FXSave_region) == 512);

static void*
fpu_alloc()
{
    struct FXSave_region* fpu = kalloc(sizeof(*fpu));
    if (fpu)
        *fpu = (struct FXSave_region){
            // Double precision, round to nearest, all exceptions masked
            .fcw    = 0x033f,
            // Round nearest, no zero flush, all exceptions masked
            .mxcsr  = 0x00001f80,
        };
    return fpu;
}

static inline void
fpu_save(void* fpu)
{
    asm volatile ("fxsave64 %0" : : "m"(*(struct FXSave_region*)fpu));
}

static inline void
fpu_load(void* fpu)
{
    asm volatile ("fxrstor64 %0" : : "m"(*(struct FXSave_region*)fpu));
}

void
x86_64_fpu_initialise()
{
    auto cr0 = read_CR(0);
    cr0 &= ~CR0_EM;         // Disable emulation
    cr0 |= CR0_MP;          // Enable monitoring
    write_CR(0, cr0);

    auto cr4 = read_CR(4);
    cr4 |= CR4_OSFXSR;      // Enable fxsave, fxrstor, sse
    write_CR(4, cr4);
}

error_code
cpu_context_initialise_user(
    struct Thread_context* context,
    physical_t page_map_top,
    usize code,
    void* stack
) {
    context->state = FLOATING;
    context->page_map_top = page_map_top;

    context->ctx = (struct Registers) {
        .cs = GDT_USER64_CODE,
        .ss = GDT_USER_DATA,
        .rflags = 0x3202, // IOPL=3, IF set
        .rip = code,
        .rsp = (uintptr_t)stack,
    };

    context->fpu_state = TRY_ALLOC(fpu_alloc());

    return 0;
}

error_code
cpu_context_clone_current(struct Thread_context* to)
{
    auto current_thread = this_tls->current_thread;
    assert(current_thread);
    assert(current_thread->state == RUNNING);
    *to = (struct Thread_context){
        .state = FLOATING,
        .ctx = *this_tls->user_context,
        .page_map_top = current_thread->page_map_top,
    };
    to->fpu_state = TRY_ALLOC(fpu_alloc());
    return 0;
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
    fpu_save(thread->fpu_state);

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
    mmu_set_address_space_opt((struct Page_map){ thread->page_map_top });
    fpu_load(thread->fpu_state);

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
