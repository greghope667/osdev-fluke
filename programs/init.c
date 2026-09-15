#include <fluke/defs/fluke.h>
#include <fluke/fluke.h>
#include <stdlib.h>

void
print_mem_regions()
{
    _fluke_forth_interpret(
        "0 get_tls_current_thread ccall1\n"
        "Thread::get_process ccall1\n"
        "Process::get_vm ccall1\n"
        "VM::print ccall1\n"
    );
}

typedef unsigned short u16;
typedef unsigned char u8;

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

const int port = 0x3f8;
static int irqd;

void serial_setup()
{
    irqd = _fluke_irq_claim(4);
    outb(port + 1, 0); // Disable interrupts
    outb(port + 4, 0xb); // IRQ, DTR, RTS
    outb(port + 1, 1); // Interrupt on RX
}

void
serial_ping(long ctx)
{
    int limit = (int)ctx;
    char id = ctx >> 32;
    for (int i=0; i<limit; i++) {
        _fluke_irq_ack_wait(irqd, 4'000'000'000);
        while (inb(port + 5) & 1) {
            outb(port, id);
            outb(port, inb(port));
        }
    }
}

static void panic()
{
    _fluke_panic("Panic from init");
}

int main()
{
    atexit(panic);
    _fluke_klog("Hello from init process");

    serial_setup();

    for (int i=0; i<3; i++) {
        long ctx = (((long)'a' + i) << 32) + 100;
        auto stack = _fluke_virtual_map(nullptr, 0x4000, PROT_READ|PROT_WRITE, 0);
        _fluke_thread_spawn(serial_ping, stack + 0x4000, ctx);
    }

    serial_ping((((long)'r') << 32) + 10);

    for (int i=0; i<3; i++)
        _fluke_forth_interpret("0 x86_64_apic_measure_frequency ccall1");

    print_mem_regions();
}
