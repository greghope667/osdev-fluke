#pragma once

#include "x86_64/cpu.h"

#ifdef __cplusplus
extern "C" {
#endif

usize syscall(Context context, struct Thread_context* thread_ctx);

#ifdef __cplusplus
} // extern C
#endif
