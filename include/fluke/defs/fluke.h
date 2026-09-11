#pragma once

#include "errno.h"                      // IWYU pragma: export
#include "seek.h"                       // IWYU pragma: export
#include "syscalls.h"                   // IWYU pragma: export
#include "virtual.h"                    // IWYU pragma: export

#define IRQ_CTL_WAIT        (1 << 0)
#define IRQ_CTL_ENABLE      (1 << 1)
