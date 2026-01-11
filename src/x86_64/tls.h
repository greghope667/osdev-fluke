#pragma once

#include "kdef.h"

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
#define kerrno (this_cpu->error)
