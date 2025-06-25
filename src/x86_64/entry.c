#include "print/console.h"
#include "klib.h"

struct Context {
    usize registers[15];
    usize isrno;
    usize error_code;
    usize rip;
    usize cs;
    usize rflags;
    usize rsp;
    usize ss;
};

#include "symbols.h"
static const struct Symbol*
symbol_of_address(void* address)
{
    const struct Symbol* match = 0;
    for (const struct Symbol* s = symbol_list; s; s = s->next) {
        if (s->address >= address)
            break;
        match = s;
    }
    return match;
}

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
        ctx->registers[0], ctx->registers[1], ctx->registers[2], ctx->registers[3]
    );
    printf(
        "    RSI %016zx  RDI %016zx  RBP %016zx  RSP %016zx\n",
        ctx->registers[4], ctx->registers[5], ctx->registers[6], ctx->rsp
    );
    printf(
        "    R8  %016zx  R9  %016zx  R10 %016zx  R11 %016zx\n",
        ctx->registers[7], ctx->registers[8], ctx->registers[9], ctx->registers[10]
    );
    printf(
        "    R12 %016zx  R13 %016zx  R14 %016zx  R15 %016zx\n",
        ctx->registers[11], ctx->registers[12], ctx->registers[13], ctx->registers[14]
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
    show_backtrace((void*)ctx->registers[6]);
    panic("Unhandled kernel exception");
}

void
exception_user_entry(u8 exception, struct Context* ctx)
{
    klog("Exception %u  error %zx  cs %zu  ss %zu:\n", exception, ctx->error_code, ctx->cs, ctx->ss);
    print_registers(ctx);
    panic("Unhandled user exception");
}

void
interrupt_entry(u8 interrupt, struct Context* ctx)
{
    klog("Interrupt %u  cs %zu  ss %zu:\n", interrupt, ctx->cs, ctx->ss);
    print_registers(ctx);
    panic("Unhandled interrupt");
}

void
syscall_entry(struct Context* ctx)
{
    klog("Syscall\n");
    print_registers(ctx);
    panic("Unhandled syscall");
}
