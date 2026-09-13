#include <fluke/fluke.h>
#include <string.h>
#include "syscall6.h"

void
_fluke_klog(const char* msg)
{
    size_t len = strlen(msg);
    while (len > 0) {
        size_t n = syscall6(
            SYSCALL_klog,
            (long)msg,
            len,
            0, 0, 0, 0
        );
        msg += n;
        len -= n;
    }
}
