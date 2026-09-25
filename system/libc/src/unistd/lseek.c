#include <unistd.h>
#include <errno.h>

#define SYSCALLV_N 3
#include "fluke/syscallv.h"

off_t
lseek(int fd, off_t offset, int whence)
{
    offset = _syscallv(SYSCALL_seek, fd, offset, whence);
    if (offset >= 0) {
        return offset;
    } else {
        errno = -offset;
        return -1;
    }
}
