#include <fluke/fluke.h>
#include <string.h>

#define SYSCALLV_N 2
#include "syscallv.h"

int
_fluke_kopen(const char* path)
{
    return _syscallv(SYSCALL_open_module, (long)path, strlen(path));
}
