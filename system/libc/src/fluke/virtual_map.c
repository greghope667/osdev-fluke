#include <fluke/fluke.h>

#define SYSCALLV_N 4
#include "syscallv.h"

void*
_fluke_virtual_map(void* address, size_t len, int prot, int flags)
{
    return (void*)_syscallv(
        SYSCALL_virtual_map,
        (long)address, len, prot, flags
    );
}
