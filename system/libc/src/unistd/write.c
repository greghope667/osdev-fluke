#include <errno.h>
#include <limits.h>
#include <unistd.h>

#define SYSCALLV_N 3
#include "fluke/syscallv.h"

 __NOTHROW
ssize_t
write(int fd, const void* buf, size_t size)
{
    if (size > INT_MAX)
        size = INT_MAX;

    long ret = _syscallv(
        SYSCALL_write,
        fd, (long)buf, size
    );

    if (ret >= 0) {
        return ret;
    } else {
        errno = -ret;
        return -1;
    }
}
