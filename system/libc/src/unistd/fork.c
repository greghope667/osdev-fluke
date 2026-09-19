#include <errno.h>
#include <unistd.h>

#define SYSCALLV_N 0
#include "fluke/syscallv.h"

pid_t
fork()
{
    int ret = _syscallv(SYSCALL_fork);
    if (ret >= 0) {
        return ret;
    } else {
        errno = -ret;
        return -1;
    }
}
