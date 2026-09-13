#include <fluke/fluke.h>
#include "syscall6.h"

void*
_fluke_virtual_map(void* address, size_t len, int prot, int flags)
{
    return (void*)syscall6(
        SYSCALL_virtual_map,
        (long)address,
        len, prot, flags,
        0, 0
    );
}
