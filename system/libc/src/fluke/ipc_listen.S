.intel_syntax noprefix

#include <fluke/defs/syscalls.h>

        .text

        .global _fluke_ipc_listen
        .type _fluke_ipc_listen, @function
_fluke_ipc_listen:
        mov     r10d, ecx                   # Channel
        mov     eax, SYSCALL_ipc_listen
        syscall
        ret
