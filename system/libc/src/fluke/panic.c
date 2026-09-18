#include <fluke/fluke.h>

#define SYSCALLV_N 0
#include "syscallv.h"

void
_fluke_panic(const char* reason)
{
    _fluke_klog("panic() called from user space:");
    _fluke_klog(reason);
    for (;;) {
        _syscallv(SYSCALL_panic);
        asm volatile ("hlt");
    }
}
