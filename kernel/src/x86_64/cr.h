#pragma once

#include "kdef.h"

#define read_CR(N) ({\
    usize _cr; \
    asm volatile ("mov %%cr" # N ", %0" : "=r"(_cr)); \
    _cr; \
})

#define write_CR(N, v) ({\
    usize _cr = v; \
    asm volatile ("mov %0, %%cr" # N : : "r"(_cr)); \
})

enum CR : u64 {
    CR0_MP              = (1 <<  1),
    CR0_EM              = (1 <<  2),

    CR4_OSFXSR          = (1 <<  9),
    CR4_OSXMMEXCPT      = (1 << 10),
};
