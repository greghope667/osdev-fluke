#pragma once

#include "kdef.h"

#ifdef __cplusplus
extern "C" {
#endif

struct Registers {
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

typedef struct Registers* Context;

enum thread_state {
    // SPAWNING,
    FLOATING,
    RUNNING,
    READY,
    WAITING,
    SLEEPING,
    CALLING,
};

struct Thread_context {
    physical_t page_map_top;
    struct Registers ctx;
    enum thread_state state;
    void* fpu_state;
};

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

error_code cpu_context_initialise_user(
    struct Thread_context* context,
    physical_t page_map_top,
    usize code,
    void* stack_top
);
error_code cpu_context_clone_current(struct Thread_context* to);
struct Thread_context* cpu_context_save();
void cpu_context_restore_and_exit(struct Thread_context* process) __attribute__((noreturn));
void cpu_exit_idle() __attribute__((noreturn));

void x86_64_fpu_initialise();

#ifdef __cplusplus
} // extern C
#endif
