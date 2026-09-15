#include <fluke/fluke.h>
#include "syscall6.h"

int
_fluke_thread_spawn(void (*func)(long), void* stack, long arg)
{
    long* stack_ptr = stack;
    *--stack_ptr = 0;

    return syscall6(
        SYSCALL_thread_spawn,
        (long)func,
        (long)stack_ptr,
        arg,
        0, 0, 0
    );
}
