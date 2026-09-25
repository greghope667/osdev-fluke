#include "file.h"
#include <errno.h>

size_t
fread(void* buf, size_t item_size, size_t nitems, FILE* f)
{
    size_t size;
    if (__builtin_umull_overflow(item_size, nitems, &size)) {
        errno = EOVERFLOW;
        return 0;
    }
    if (size == 0)
        return 0;
    return __libc_read(f, buf, size) / item_size;
}
