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

struct Cpu {
    struct Cpu* self;
    usize kernel_stack;
    usize user_stack;
    u8 lapic_id;
};

#define this_cpu ((__seg_gs struct Cpu*)0)

inline struct Cpu*
get_cpu() { return this_cpu->self; }

extern struct Cpu* (*cpu_array)[];
extern int cpu_count;

void x86_64_cpu_create_tls(u8 lapic_id, usize kernel_stack);
