#include "file.h"
#include <unistd.h>

int
fseeko(FILE* f, off_t offset, int whence)
{
    return lseek(f->fd, offset, whence) >= 0 ? 0 : -1;
}

int
fseek(FILE*, long, int) __attribute__((alias("fseeko")));
