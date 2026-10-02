#include <sys/mman.h>

#include <errno.h>

#define SYSCALLV_N 2
#include "fluke/syscallv.h"

int
munmap(void* addr, size_t len)
{
    int r = _syscallv(SYSCALL_virtual_unmap, (long)addr, len);
    if (r >= 0) {
        return 0;
    } else {
        errno = -r;
        return -1;
    }
}
