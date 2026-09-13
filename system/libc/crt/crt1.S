.intel_syntax noprefix

#include <fluke/defs/syscalls.h>

# Entry point, immediately after exec completes
# The stack must be laid out as described by the AMD64 System V ABI
# 'atexit' pointer rdx is not used/supported.

        .text

        .global     _start
_start:
        .cfi_startproc
        .cfi_undefined  rip             # Prevent dwarf unwinding
        xor     ebp, ebp                # End of stack frame ptr
        mov     rdi, rsp                # argc/argv pointer
        and     rsp, -16                # Align stack just in case
        push    rbp

        lea     rsi, main
        lea     rdx, _init
        lea     rcx, _fini
        call    __libc_start_main
        mov     rdi, rax                # Return status
        mov     rbx, rax
        call    exit

        # exit() should not return.
        # This is paranoid code for if the user breaks something
        mov     rdi, rbx
        call    _exit
        .cfi_endproc


        .global     _Exit
_Exit:
        .global     _exit
_exit:
1:      mov     eax, SYSCALL_process_exit
        syscall
        hlt
        jmp     1b
