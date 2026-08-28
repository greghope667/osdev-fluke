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

unsigned long
strlen(const char* str)
{
    unsigned long n = 0;
    while (str[n]) n++;
    return n;
}

void
forth(const char* str)
{
    syscall6(SYSCALL_forth_interpret, (long)str, strlen(str)+1, 0, 0, 0, 0);
}

int main()
{
    for (int i=0; i<3; i++)
        syscall6(SYSCALL_virtual_map, (0x60+i)<<12, 0x4000, PROT_READ|PROT_WRITE, MAP_FIXED, 0, 0);
    *(int*)0x60000 = 0xaabbccdd;
    // syscall6(SYSCALL_nsleep, 5'000'000'000, 0, 0, 0, 0, 0);

    for (int i=0; i<3; i++)
        forth("0 x86_64_apic_measure_frequency ccall1");

    forth(
        "0 get_tls_current_thread ccall1\n"
        "Thread::get_process ccall1\n"
        "Process::get_vm ccall1\n"
        "VM::print ccall1\n"
    );

    static const char panic[] = ": abort0 parse drop panic ccall1 ; abort0 abort from init";
    forth(panic);

    *(volatile int*)0x1234567890 = 12;
}
