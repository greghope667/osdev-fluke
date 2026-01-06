#pragma once

#include "kdef.h"

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

struct Process;

void cpu_context_initialise_user(struct Context* context, usize code, usize stack);
void cpu_context_save();
void cpu_context_restore_and_exit(struct Process* process);

struct Cpu {
    struct Cpu* self;
    usize kernel_stack;
    usize user_stack;
    u8 lapic_id;
    int error;
    struct Process* process;
    struct Context* user_context;
};

#define this_cpu ((__seg_gs struct Cpu*)0)
#define errno (this_cpu->error)

inline struct Cpu*
get_cpu() { return this_cpu->self; }

extern struct Cpu* (*cpu_array)[];
extern int cpu_count;

void x86_64_cpu_create_tls(u8 lapic_id, usize kernel_stack);
