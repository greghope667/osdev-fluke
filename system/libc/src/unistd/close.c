#include <errno.h>
#include <unistd.h>

#define SYSCALLV_N 1
#include "fluke/syscallv.h"

int
close(int fd)
{
    int n = _syscallv(SYSCALL_close, fd);
    if (n >= 0) {
        return 0;
    } else {
        errno = -n;
        return -1;
    }
}
