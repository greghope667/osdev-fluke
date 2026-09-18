#include <fluke/fluke.h>
#include <string.h>

#define SYSCALLV_N 2
#include "syscallv.h"

void
_fluke_klog(const char* msg)
{
    size_t len = strlen(msg);
    while (len > 0) {
        size_t n = _syscallv(SYSCALL_klog, (long)msg, len);
        msg += n;
        len -= n;
    }
}
