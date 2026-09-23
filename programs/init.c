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
            outb(port, id);
            outb(port, inb(port));
        }
    }
}

void ipc_client(long fd)
{
    for (;;) {
        _fluke_nsleep(750'000'000);
        char buf[5] = "tx"; auto ret = _fluke_ipc_call(
            fd, (long)buf, sizeof(buf),
            (1 << IPC_CLASS_SHIFT) | IPC_CALL_RXSTR | IPC_CALL_TXSTR);
        for (int i=0; i<ret.second; i++) {
            outb(port, buf[i]);
        }
    }
}

int ipc_server_callback(long, long aptr, long alen, int, long, long)
{
    for (int i=0; i<alen; i++) {
        outb(port, ((char*)aptr)[i]);
    }
    return _fluke_ipc_respond(0, (long)"rx", 2, IPC_CALL_RXSTR);
}

void ipc_server(long handle)
{
    for (;;) {
        char buf[10] = {};
        _fluke_ipc_listen(handle, (long)buf, sizeof(buf), 0, ipc_server_callback);
    }
}

void create_ipcs()
{
    __int8_t map[] = {
        [IPC_TRANSFER_REGISTER] = 0,
        [IPC_TRANSFER_SMALLSTR] = 0,
    };
    auto pair = _fluke_ipc_create(map, sizeof(map));
    if (fork() == 0) {
        ipc_client(pair.first);
    }
    auto stack = _fluke_virtual_map(nullptr, 0x4000, PROT_READ|PROT_WRITE, 0);
    _fluke_thread_spawn(ipc_server, stack + 0x2000, pair.second);
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

__attribute__((constructor(50))) void construct50() { _fluke_klog(__PRETTY_FUNCTION__); }
__attribute__((constructor(150))) void construct150() { _fluke_klog(__PRETTY_FUNCTION__); }
__attribute__((destructor(50))) void destruct50() { _fluke_klog(__PRETTY_FUNCTION__); }
__attribute__((destructor(150))) void destruct150() { _fluke_klog(__PRETTY_FUNCTION__); }

int main()
{
    test_fault();

    _fluke_klog("Hello from init process");

    serial_setup();

    for (int i=0; i<3; i++)
        create_ipcs();

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
