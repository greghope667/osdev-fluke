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

1:      mov     eax, SYSCALL_forth_interpret
        lea     rdi, [forth_code]
        mov     rsi, forth_code_end - forth_code
        syscall

        mov     eax, SYSCALL_nsleep
        mov     edi, 1500000000
        // syscall

        mov     eax, SYSCALL_open_module
        lea     rdi, [rip + .Lmodule_name]
        mov     esi, .Lmodule_name_end - .Lmodule_name
        syscall
        mov     ebx, eax

        mov     eax, SYSCALL_read
        mov     edi, ebx
        lea     rsi, [rsp - 16]
        mov     edx, 16
        syscall

        mov     r8, [rsp - 16]
        mov     r9, [rsp - 8]

        nop
        mov     qword ptr [0x12345678], 1
        hlt

.Lmodule_name:
        .ascii  "/boot/limine/limine.conf"
.Lmodule_name_end:

        .balign 8
pid0_size:
        .quad   . - pid0_code

forth_code:
        .ascii  ".( hello from pid0! ) cr "
        .ascii  "0 pmm_print_info ccall1 drop "
        .ascii  "0 nanoseconds ccall1 "
forth_code_end:
