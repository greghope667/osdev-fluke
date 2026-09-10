#pragma once

#include "kdef.h"

#ifdef __cplusplus
extern "C" {
#endif

struct Thread_context;
struct Queue;

void schedule_ready(struct Thread_context* thread_ctx);
void schedule_nanosleep(struct Thread_context* thread_ctx, isize wait_ns);
void schedule_queue_with_timeout(
    struct Thread_context* thread_ctx,
    struct Queue* queue,
    isize wait_ns,
    error_code timeout_return_value
);
void schedule_wake(struct Thread_context* thread_ctx, usize return_value);
void schedule_wake_all(struct Queue* queue, usize return_value);
void schedule();
void schedule_or_exit() __attribute__((noreturn));

#ifdef __cplusplus
} // extern C
#endif
