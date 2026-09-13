#include <fluke/fluke.h>
#include <string.h>
#include "syscall6.h"

long
_fluke_forth_interpret(const char* code_str)
{
    return syscall6(
        SYSCALL_forth_interpret,
        (long)code_str,
        strlen(code_str) + 1,
        0, 0, 0, 0
    );
}
