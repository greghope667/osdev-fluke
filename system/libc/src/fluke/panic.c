#include <fluke/fluke.h>
#include "syscall6.h"

void
_fluke_panic(const char* reason)
{
    _fluke_klog("panic() called from user space:");
    _fluke_klog(reason);
    for (;;) {
        syscall6(
            SYSCALL_panic,
            0, 0, 0, 0, 0, 0
        );
        asm volatile ("hlt");
    }
}
