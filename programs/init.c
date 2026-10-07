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

// ### Serial IO / Mini TTY ###

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

static void
tty_writechar(int ch)
{
    while (!(inb(port + 5) & 0x40))
        __builtin_ia32_pause();
    outb(port, ch);
}

static int
tty_read(long, long, long alen, int, long, long)
{
    char buf[256];
    if ((size_t)alen > sizeof(buf))
        alen = sizeof(buf);

    for (long i=0; i<alen; i++) {
        while (!(inb(port + 5) & 0x01))
            _fluke_irq_ack_wait(irqd, 1'000'000'000);
        char ch = inb(port);
        if (ch == '\r') ch = '\n';
        tty_writechar(ch);
        buf[i] = ch;
    }
    return _fluke_ipc_respond(alen, (long)buf, alen, IPC_CALL_RXSTR);
}

__NORETURN
static void
tty_listen_read(long ipc_handle)
{
    for (;;)
        _fluke_ipc_listen(ipc_handle, 0, 0, 1, tty_read);
}

static int
tty_write(long, long aptr, long alen, int, long, long)
{
    for (long i=0; i<alen; i++) {
        int ch = ((char*)aptr)[i];
        tty_writechar(ch);
    }
    return _fluke_ipc_respond(alen, 0, 0, 0);
}

__NORETURN
static void
tty_listen_write(long ipc_handle)
{
    char buf[256];
    for (;;)
        _fluke_ipc_listen(ipc_handle, (long)buf, sizeof(buf), 2, tty_write);
}

void serial_setup()
{
    irqd = _fluke_irq_claim(4);
    outb(port + 1, 0);      // Disable interrupts
    outb(port + 4, 0xb);    // IRQ, DTR, RTS
    outb(port + 1, 1);      // Interrupt on RX
}

static int
tty_setup()
{
    serial_setup();
    u8 map[] = {
        [IPC_TRANSFER_READ]  = 1,
        [IPC_TRANSFER_WRITE] = 2,
    };
    auto pair = _fluke_ipc_create(map, sizeof(map));
    if (fork() > 0)
        return pair.first;

    {
        size_t stacklen = 4096 * 4;
        void* stack = _fluke_virtual_map(0, stacklen, PROT_READ|PROT_WRITE, 0);
        _fluke_thread_spawn(tty_listen_read, stack + stacklen, pair.second);
    }
    {
        tty_listen_write(pair.second);
    }
}

// ### Test Programs ###

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
    char devminor[8];
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

#include "khashl.h"
struct file_contents { void* data; size_t size; };
KHASHL_MAP_INIT(KH_LOCAL, map_fs, map_fs, char*, struct file_contents, kh_hash_str, kh_eq_str)

void malloc_stats(void);

static void
ls_ramdisk()
{
    int ret;
    map_fs* h = map_fs_init();

    int fd = _fluke_kopen("/initrd.tar");
    FILE *f = fdopen(fd, "r");
    printf("fd %i file %p\n", fd, f);

    size_t n;
    struct ustar_record record;
    while ((n = fread(&record, 512, 1, f))) {
        if (!record.name[0])
            break;

        auto length = read_octal(record.size);
        int blocks = (length + 511) / 512;
        if (length) {
            struct file_contents contents = {
                .data = malloc(length),
                .size = length,
            };
            fread(contents.data, length, 1, f);

            auto name = strndup(record.name, sizeof(record.name));
            auto i = map_fs_put(h, name, &ret);
            kh_val(h, i) = contents;
        }
        fseek(f, blocks * 512 - length, SEEK_CUR);
    }

    {
        unsigned x;
        kh_foreach(h, x)
            printf("%8zu %s\n", kh_val(h, x).size, kh_key(h, x));
    }

    malloc_stats();

    fclose(f);
    {
        unsigned x;
        kh_foreach(h, x) {
            free(kh_key(h, x));
            free(kh_val(h, x).data);
        }
    }
    map_fs_destroy(h);
}

static void
hello_process()
{
    if (fork() == 0) {
        int fd = _fluke_kopen("/hello");
        static char* const args[] = {
            "hello", "123", "abc", 0
        };
        static char* const env[] = {
            "EDITOR=vim", 0
        };
        fexecve(fd, args, env);
    }
    _fluke_nsleep(1'000'000'000);
}

static long
nanoseconds()
{
    return _fluke_forth_interpret("0 nanoseconds ccall1").second;
}

static void
time()
{
    size_t ns = nanoseconds();
    printf("time %6lu.%09lu\n", ns / 1'000'000'000, ns % 1'000'000'000);
}

int main()
{
    int tty = tty_setup();
    dup2(tty, 0);
    dup2(tty, 1);
    dup2(tty, 2);
    printf("tty = %i\n", tty);

    test_fault();

    hello_process();

    ls_ramdisk();

    puts("Hello from init process");
    printf("printf %s %p %d %f\n", __PRETTY_FUNCTION__, main, 123456, 123.456);

    for (;;) {
        fwrite("> ", 1, 2, stdout);
        char* line = 0;
        size_t n = 0;
        auto len = getline(&line, &n, stdin);
        if (len == -1)
            break;
        if (line[len-1] == '\n') line[len-1] = 0;
        printf("got line > %s < len %zu\n", line, len);

        if (strcmp(line, "ls") == 0) {
            ls_ramdisk();
        } else if (strcmp(line, "mf") == 0) {
            malloc_stats();
        } else if (strcmp(line, "time") == 0) {
            time();
        } else if (strcmp(line, "exit") == 0) {
            break;
        } else if (strcmp(line, "boom") == 0) {
            _fluke_forth_interpret("0 mmu_set_address_space ccall1");
        }

        free(line);
    }

    for (int i=0; i<3; i++)
        _fluke_forth_interpret("0 x86_64_apic_measure_frequency ccall1");

    print_mem_regions();
}
