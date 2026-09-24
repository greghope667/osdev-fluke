#include "file.h"
#include <limits.h>

size_t
__libc_read(FILE* f, void* data, size_t size)
{
    long ctx = f->fd;
    auto readfn = (int(*)(long, void*, size_t))(void*)f->readfn;

    for (size_t remaining = size; remaining > 0; ) {
        int n = remaining > INT_MAX ? INT_MAX : remaining;
        n = readfn(ctx, data, (unsigned)n);

        if (n < 0) {
            f->error = true;
            return size - remaining;
        } else if (n == 0) {
            f->eof = true;
            return size - remaining;
        }
        remaining -= n;
        data += n;
    }
    return size;
}
