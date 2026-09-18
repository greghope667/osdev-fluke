#define SYSCALLV_N 1
#include "syscallv.h"

void
_fluke_nsleep(long ns)
{
    _syscallv(SYSCALL_nsleep, ns);
}
