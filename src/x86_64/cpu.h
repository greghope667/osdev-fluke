#pragma once

#include "kdef.h"
#include "tls.h"

struct Context {
    union {
        usize registers[15];
        struct {
            usize rax, rbx, rcx, rdx;
            usize rsi, rdi, rbp;
            usize r8 , r9 , r10, r11;
            usize r12, r13, r14, r15;
        };
    };
    usize isrno;
    usize error_code;
    usize rip;
    usize cs;
    usize rflags;
    usize rsp;
    usize ss;
};

typedef struct Context* Context;

#define CTX_SYS_OP(ctx) ((ctx)->rax)
#define CTX_SYS_A0(ctx) ((ctx)->rdi)
#define CTX_SYS_A1(ctx) ((ctx)->rsi)
#define CTX_SYS_A2(ctx) ((ctx)->rdx)
#define CTX_SYS_A3(ctx) ((ctx)->r10)
#define CTX_SYS_A4(ctx) ((ctx)->r8)
#define CTX_SYS_A5(ctx) ((ctx)->r9)
#define CTX_SYS_R0(ctx) ((ctx)->rax)
#define CTX_SYS_R1(ctx) ((ctx)->rdx)
#define CTX_SYS_PC(ctx) ((ctx)->rip)

void cpu_context_initialise_user(Context context, usize code, usize stack);
struct Process* cpu_context_save();
void cpu_context_restore_and_exit(struct Process* process) __attribute__((noreturn));
void cpu_exit_idle() __attribute__((noreturn));

inline struct Cpu*
get_cpu() { return this_cpu->self; }

extern struct Cpu* (*cpu_array)[];
extern int cpu_count;

void x86_64_cpu_create_tls(u8 lapic_id, usize kernel_stack);
