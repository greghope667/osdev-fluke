#include <fluke/fluke.h>

#define SYSCALLV_N 3
#include "syscallv.h"

int
_fluke_thread_spawn(void (*func)(long), void* stack, long arg)
{
    long* stack_ptr = stack;
    if (((long)stack_ptr & 0xf) == 0)
        *--stack_ptr = 0;

    return _syscallv(
        SYSCALL_thread_spawn,
        (long)func, (long)stack_ptr, arg
    );
}
