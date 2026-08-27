#include "fluke.h"

asm (
    ".global    _start\n"
    ".pushsection .text\n"
"_start:\n"
    "push   %rax\n"
    "call   main\n"
"1: "
    "hlt\n"
    "jmp    1b\n"
    ".popsection\n"
);

inline long
syscall6(long rax, long a1, long a2, long a3, long a4, long a5, long a6)
{
    register long r9 asm("r9") = a6;
    register long r8 asm("r8") = a5;
    register long r10 asm("r10") = a4;
    asm volatile (
        "syscall"
        : "+a"(rax), "+d"(a3)
        : "D"(a1), "S"(a2), "r"(r10), "r"(r8), "r"(r9)
        : "memory", "rcx", "r11"
    );
    return rax;
}

int main()
{
    syscall6(SYSCALL_virtual_map, 0x60000, 0x4000, PROT_READ|PROT_WRITE, MAP_FIXED, 0, 0);

    static const char panic[] = ": abort0 parse drop panic ccall1 ; abort0 abort from init";
    syscall6(SYSCALL_forth_interpret, (long)panic, sizeof(panic), 0, 0, 0, 0);
}
