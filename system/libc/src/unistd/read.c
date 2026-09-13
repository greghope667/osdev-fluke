#include <errno.h>
#include <limits.h>
#include <unistd.h>
#include "syscall6.h"

 __NOTHROW
ssize_t
read(int fd, void* restrict buf, size_t size)
{
    if (size > INT_MAX)
        size = INT_MAX;

    long ret = syscall6(
        SYSCALL_read,
        fd,
        (intptr_t)buf,
        size,
        0, 0, 0
    );

    if (ret >= 0) {
        return ret;
    } else {
        errno = -ret;
        return -1;
    }
}
