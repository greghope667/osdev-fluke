#include <fcntl.h>
#include "libc_impl.h"

static int
_open(const char* path, int oflag, mode_t mode)
{
    return openat(__libc_working_dirfd, path, oflag, mode);
}

int
open(const char* path, int oflag, ...) __attribute__((alias("_open")));

int
creat(const char* path, mode_t mode)
{
    return _open(path, O_WRONLY|O_CREAT|O_TRUNC, mode);
}
