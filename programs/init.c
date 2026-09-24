#include <fluke/defs/fluke.h>
#include <fluke/fluke.h>
#include <unistd.h>

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
            char buf[2] = { id, inb(port) };
            fwrite(buf, 1, 2, stderr);
        }
    }
}

__attribute__((destructor(0)))
static void panic()
{
    _fluke_panic("Panic from init");
}

static void test_fault()
{
    int err = _fluke_ipc_create((void*)0x1000, 1).first;
    if (err != -EFAULT)
        _fluke_panic("test_fault() failed");
}

static int
stdout_write_callback(long, long aptr, long alen, int, long, long)
{
    for (long i=0; i<alen; i++) {
        while (!(inb(port + 5) & 0x40))
            __builtin_ia32_pause();
        auto ch = ((char*)aptr)[i];
        outb(port, ch);
    }
    return _fluke_ipc_respond(alen, 0, 0, 0);
}

static void
setup_stdout()
{
    u8 map[] = {
        [IPC_TRANSFER_WRITE] = 1,
    };
    auto pair = _fluke_ipc_create(map, sizeof(map));
    dup2(pair.first, 1);
    dup2(pair.first, 2);
    if (fork() > 0)
        return;

    char buf[128];
    for (;;) {
        _fluke_ipc_listen(pair.second, (long)buf, sizeof(buf), 1, stdout_write_callback);
    }
}

int main()
{
    setup_stdout();

    serial_setup();
    test_fault();

    puts("Hello from init process");
    printf("printf %s %p %d %f\n", __PRETTY_FUNCTION__, main, 123456, 123.456);

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
