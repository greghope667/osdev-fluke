#include <fluke/fluke.h>
#include <string.h>

#define SYSCALLV_O 1
#define SYSCALLV_N 2
#include "syscallv.h"

struct _fluke_lpair
_fluke_forth_interpret(const char* code_str)
{
    long tos;
    auto n = _syscallv(
        &tos,
        SYSCALL_forth_interpret,
        (long)code_str, strlen(code_str) + 1
    );
    return (struct _fluke_lpair){ n, tos };
}
