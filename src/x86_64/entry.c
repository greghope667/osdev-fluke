#include "print/console.h"
#include "klib.h"
#include "user/schedule.h"
#include "user/syscall.h"
#include "x86_64/apic.h"
#include "cpu.h"
#include "symbols.h"

static void
print_registers(struct Context* ctx)
{
    printf("    RIP %016zx  ", ctx->rip);
    {
        auto sym = symbol_of_address((void*)ctx->rip);
        if (sym) {
            printf("%s+%zx\n", sym->name, (void*)ctx->rip - sym->address);
        } else {
            putchar('\n');
        }
    }
    printf(
        "    RAX %016zx  RBX %016zx  RCX %016zx  RDX %016zx\n",
        ctx->rax, ctx->rbx, ctx->rcx, ctx->rdx
    );
    printf(
        "    RSI %016zx  RDI %016zx  RBP %016zx  RSP %016zx\n",
        ctx->rsi, ctx->rdi, ctx->rbp, ctx->rsp
    );
    printf(
        "    R8  %016zx  R9  %016zx  R10 %016zx  R11 %016zx\n",
        ctx->r8 , ctx->r9 , ctx->r10, ctx->r11
    );
    printf(
        "    R12 %016zx  R13 %016zx  R14 %016zx  R15 %016zx\n",
        ctx->r12, ctx->r13, ctx->r14, ctx->r15
    );
}

void
exception_kernel_entry(u8 exception, struct Context* ctx)
{
    console_setcolor(COLOR_BRIGHT_RED, COLOR_BLACK);
    klog("Exception\nExn %u  error 0x%zx  cs %zu  ss %zu:\n", exception, ctx->error_code, ctx->cs, ctx->ss);
    if (exception == 14) {
        usize cr2;
        asm ("mov   %%cr2, %0" : "=r"(cr2));
        printf("Page fault address: %zx\n", cr2);
    }
    print_registers(ctx);
    show_backtrace((void*)ctx->rbp);
    panic("Unhandled kernel exception");
}

void
exception_user_entry(u8 exception, struct Context* ctx)
{
    console_setcolor(COLOR_BRIGHT_MAGENTA, COLOR_BLACK);
    klog("Exception %u  error %zx  cs %zu  ss %zu:\n", exception, ctx->error_code, ctx->cs, ctx->ss);
    if (exception == 14) {
        usize cr2;
        asm ("mov   %%cr2, %0" : "=r"(cr2));
        printf("Page fault address: %zx\n", cr2);
    }
    print_registers(ctx);
    panic("Unhandled user exception");
}

void
interrupt_entry(u8 interrupt, struct Context* ctx)
{
    klog("Interrupt %u  cs %zu  ss %zu:\n", interrupt, ctx->cs, ctx->ss);
    print_registers(ctx);
    if (interrupt < 254)
        panic("Unhandled interrupt");
    x86_64_apic_send_eoi();
    this_cpu->user_context = ctx;
    schedule();
}

void
syscall_entry(struct Context* ctx)
{
    klog("Syscall %zx: (%zx, %zx, %zx, %zx, %zx, %zx)\n",
        CTX_SYS_OP(ctx),
        CTX_SYS_A0(ctx), CTX_SYS_A1(ctx), CTX_SYS_A2(ctx),
        CTX_SYS_A3(ctx), CTX_SYS_A4(ctx), CTX_SYS_A5(ctx)
    );
    print_registers(ctx);
    this_cpu->user_context = ctx;
    CTX_SYS_R0(ctx) = syscall(ctx);

    // If thread has been scheduled away, syscall() should not return
    assert(this_cpu->process != nullptr);
}
