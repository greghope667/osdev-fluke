#include <fcntl.h>
#include <fluke/fluke.h>
#include <fluke/defs/ipc-client.h>
#include <errno.h>
#include <string.h>

static int
_openat(int dirfd, const char* path, int oflag, mode_t mode)
{
    int fd = _fluke_ipc_call(
        dirfd,
        (long)path, strlen(path),
        IPC_openat,
        oflag, mode
    ).first;
    if (fd >= 0) {
        if (!(oflag & O_CLOEXEC)) {
            // TODO: clear cloexec on fd
        }
        return fd;
    } else {
        errno = -fd;
        return -1;
    }
}

int
openat(int, const char*, int, ...) __attribute__((alias("_openat")));
