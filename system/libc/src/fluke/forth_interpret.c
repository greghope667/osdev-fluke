#include <fluke/fluke.h>
#include <string.h>

#define SYSCALLV_N 2
#include "syscallv.h"

long
_fluke_forth_interpret(const char* code_str)
{
    return _syscallv(
        SYSCALL_forth_interpret,
        (long)code_str, strlen(code_str) + 1
    );
}
