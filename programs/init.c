#include <fluke/defs/fluke.h>

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

static inline long
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

void
print_mem_regions()
{
    forth(
        "0 get_tls_current_thread ccall1\n"
        "Thread::get_process ccall1\n"
        "Process::get_vm ccall1\n"
        "VM::print ccall1\n"
    );
}

typedef unsigned short u16;
typedef unsigned char u8;

static void
print(const char* str)
{
    auto len = strlen(str);
    while (len > 0) {
        auto n = syscall6(SYSCALL_klog, (long)str, len, 0, 0, 0, 0);
        len -= n;
        str += n;
    }
}

static inline void
outb(u16 addr, u8 val)
{
    asm volatile ("outb\t%1, %0" : : "Nd"(addr), "a"(val));
}

static inline u8
inb(u16 addr)
{
    u8 val;
    asm volatile ("inb\t%1, %0" : "=a"(val) : "Nd"(addr));
    return val;
}

void serial_setup()
{
    int fd = syscall6(SYSCALL_claim_irq, 4, 0, 0, 0, 0, 0);
    const int port = 0x3f8;
    outb(port + 1, 0); // Disable interrupts
    outb(port + 4, 0xb); // IRQ, DTR, RTS
    outb(port + 1, 1); // Interrupt on RX

    for (int i=0; i<100; i++) {
        syscall6(SYSCALL_objctl, fd, IRQ_CTL_ENABLE|IRQ_CTL_WAIT, 1'000'000'000, 0, 0, 0);
        while (inb(port + 5) & 1) {
            outb(port, inb(port));
        }
    }
}


int main()
{
    print("Hello from init process\n");

    for (int i=0; i<3; i++) {
        syscall6(SYSCALL_virtual_map, (0x60+i)<<12, 0x4000, PROT_READ|PROT_WRITE, 0, 0, 0);
        syscall6(SYSCALL_virtual_map, 0, 0x8000, PROT_READ|PROT_WRITE, 0, 0, 0);
    }

    serial_setup();

    for (int i=0; i<3; i++)
        forth("0 x86_64_apic_measure_frequency ccall1");

    print_mem_regions();

    syscall6(SYSCALL_panic, 0, 0, 0, 0, 0, 0);
}
