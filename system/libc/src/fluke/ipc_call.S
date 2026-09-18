.intel_syntax noprefix

#include <fluke/defs/syscalls.h>

        .text

        .global _fluke_ipc_call
        .type _fluke_ipc_call, @function
_fluke_ipc_call:
        mov     r10d, ecx                   # Mode
        mov     eax, SYSCALL_ipc_call
        syscall
        ret
