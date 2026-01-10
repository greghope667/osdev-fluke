.intel_syntax noprefix
#include "fluke.h"

        .global pid0_code
        .global pid0_size

        .section .rodata

        .balign 8
pid0_code:
        nop
        mov     eax, SYSCALL_nop
        mov     ebx, 1
        mov     ecx, 2
        mov     edx, 3
        mov     esi, 4
        mov     edi, 5
        mov     ebp, 6
        mov     r8d, 8
        mov     r9d, 9
        mov     r10d, 10
        mov     r11d, 11
        mov     r12d, 12
        mov     r13d, 13
        mov     r14d, 14
        mov     r15d, 15
        syscall

        mov     eax, SYSCALL_forth_interpret
        lea     rdi, [forth_code]
        mov     rsi, forth_code_end - forth_code
        syscall

        mov     eax, SYSCALL_forth_interpret
        syscall

        nop
        mov     qword ptr [0x12345678], 1
        hlt

        .balign 8
pid0_size:
        .quad   . - pid0_code

forth_code:
        .ascii  ".( hello from pid0! ) cr "
        .ascii  "0 pmm_print_info ccall1 drop "
        .ascii  "0 nanoseconds ccall1 "
forth_code_end:
