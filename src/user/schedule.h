#pragma once

#include "kdef.h"
#include "x86_64/cpu.h"

#ifdef __cplusplus
extern "C" {
#endif

void schedule_ready(struct Thread_context* thread_ctx);
void schedule_nanosleep(struct Thread_context* thread_ctx, u64 wait_ns);
void schedule();
void schedule_or_exit() __attribute__((noreturn));

#ifdef __cplusplus
} // extern C
#endif
