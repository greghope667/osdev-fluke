#include "file.h"
#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

static int
parse_mode_string(const char* mode)
{
    char first = mode[0];
    int flags = 0;
    bool plus;

    for (size_t i=1;; i++) {
        switch (mode[i]) {
        case '+':
            if (plus) goto fail;
            plus = true;
            break;

        case 'x':
            if (flags & O_EXCL)
                goto fail;
            flags |= O_EXCL;
            break;

        case 'e':
            if (flags & O_CLOEXEC)
                goto fail;
            flags |= O_CLOEXEC;
            break;

        case 'b':
            break;

        case 0:
            goto string_end;

        default:
            goto fail;
        }
    }

string_end:
    switch (first) {
    case 'r':
        flags |= plus ? O_RDWR : O_RDONLY;
        break;

    case 'w':
        flags |= (plus ? O_RDWR : O_WRONLY) | O_CREAT | O_TRUNC;
        break;

    case 'a':
        flags |= (plus ? O_RDWR : O_WRONLY) | O_CREAT | O_APPEND;
        break;

    default:
        goto fail;
    }


    return flags;

fail:
    errno = EINVAL;
    return 0;
}

static long
_badf()
{
    errno = EBADF;
    return -1;
}

FILE*
__libc_fdopen(int fd, int oflags)
{
    FILE* f = __libc_alloc_stream();
    if (!f)
        return nullptr;

    *f = (FILE){
        .fd = fd,
        .writefn = (void*)_badf,
        .readfn = (void*)_badf,
        .closefn = (void*)close,
    };

    if (oflags & (O_RDONLY|O_RDWR))
        f->readfn = (void*)read;
    if (oflags & (O_WRONLY|O_RDWR))
        f->writefn = (void*)write;

    return f;
}

FILE*
fdopen(int fd, const char* mode)
{
    int oflags = parse_mode_string(mode);
    return oflags ? __libc_fdopen(fd, oflags) : nullptr;
}

FILE*
fopen(const char* path, const char* mode)
{
    int oflags = parse_mode_string(mode);
    if (oflags == 0)
        return nullptr;

    int fd = openat(__libc_working_dirfd, path, oflags, 0666);
    if (fd == -1)
        return nullptr;

    return __libc_fdopen(fd, oflags);
}
