#include "file.h"
#include <limits.h>

size_t
__libc_write(FILE* f, const void* data, size_t size)
{
    long ctx = f->fd;
    auto writefn = (int(*)(long, const void*, size_t))(void*)f->writefn;

    for (size_t remaining = size; remaining > 0; ) {
        int n = remaining > INT_MAX ? INT_MAX : remaining;
        n = writefn(ctx, data, (unsigned)n);

        if (n <= 0) {
            f->error = true;
            return size - remaining;
        }
        remaining -= n;
        data += n;
    }
    return size;
}
