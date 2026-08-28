#pragma once

#include "kdef.h"

#ifdef __cplusplus
extern "C" {
#endif

struct Tls {
    struct Tls* self;
    usize kernel_stack;
    usize user_stack;
    u8 lapic_id;
    struct Thread_context* current_thread;
    struct Registers* user_context;
};

#define this_tls ((__seg_gs struct Tls*)0)

struct Tls* get_tls();
struct Thread_context* get_tls_current_thread();
void x86_64_cpu_create_tls(u8 lapic_id, usize kernel_stack);

#ifdef __cplusplus
} // extern C
#endif
