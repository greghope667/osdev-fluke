#include <assert.h>
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

struct ustar_record {
    char name[100];
    char mode[8];
    char uid[8];
    char gid[8];
    char size[12];
    char mtime[12];
    char chksum[8];
    char typeflag;
    char linkname[100];
    char magic[6];
    char version[2];
    char uname[32];
    char gname[32];
    char devmajor[8];
    char devmidor[8];
    char prefix[155];
};

static size_t
read_octal(const char* s)
{
    size_t value = 0;
    while (*s)
        value = value * 8 + *s++ - '0';
    return value;
}

#include "khash.h"
struct file_contents { void* data; size_t size; };
KHASH_MAP_INIT_STR(fs, struct file_contents)

void malloc_stats(void);

static void
ls_ramdisk()
{
    malloc_stats();
    int ret;
    khash_t(fs)* fs = kh_init(fs);

    int fd = _fluke_kopen("/initrd.tar");
    FILE *f = fdopen(fd, "r");

    size_t n;
    struct ustar_record record;
    while ((n = fread(&record, 512, 1, f))) {
        if (!record.name[0])
            break;

        auto length = read_octal(record.size);
        printf("%c %8zu %s\n", record.typeflag, length, record.name);
        int blocks = (length + 511) / 512;
        if (length) {
            struct file_contents contents = {
                .data = malloc(length),
                .size = length,
            };
            fread(contents.data, length, 1, f);

            auto name = strndup(record.name, sizeof(record.name));
            auto i = kh_put(fs, fs, name, &ret);
            kh_val(fs, i) = contents;
        }
        fseek(f, blocks * 512 - length, SEEK_CUR);
    }

    {
        const char* key; struct file_contents value;
        kh_foreach(fs, key, value, {
            printf("%s %zu\n", key, value.size);
        });
    }
    malloc_stats();
}

int main()
{
    setup_stdout();

    serial_setup();
    test_fault();

    ls_ramdisk();

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
