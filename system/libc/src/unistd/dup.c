#include <errno.h>
#include <unistd.h>

#define SYSCALLV_N 3
#include "fluke/syscallv.h"

static int
_dup(int oldfd, int newfd, int flags)
{
    newfd = _syscallv(SYSCALL_dup, oldfd, newfd, flags);
    if (newfd >= 0) {
        return newfd;
    } else {
        errno = -newfd;
        return -1;
    }
}

int
dup(int oldfd)
{
    return _dup(oldfd, -1, 0);
}

int
dup2(int oldfd, int newfd)
{
    if (oldfd == newfd)
        return newfd;
    if (newfd >= 0)
        return _dup(oldfd, newfd, 0);
    errno = EBADF;
    return -1;
}

int
dup3(int oldfd, int newfd, int flags)
{
    if (newfd >= 0)
        _dup(oldfd, newfd, flags);
    errno = EBADF;
    return -1;
}
