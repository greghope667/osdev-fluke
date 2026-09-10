#pragma once

#include "x86_64/cpu.h"

#ifdef __cplusplus
extern "C" {
#endif

usize syscall(Context context, struct Thread_context* thread_ctx);
const char* syscall_get_name(Context context);

#ifdef __cplusplus
} // extern C
#endif
