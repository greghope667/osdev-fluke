#pragma once

#include "kdef.h"
#include "process.h"

void schedule_ready(struct Process* process);
void schedule_nanosleep(struct Process* process, u64 wait_ns);
void schedule();
void schedule_or_exit() __attribute__((noreturn));
