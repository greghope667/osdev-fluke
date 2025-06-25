#include "alloc.h"
#include "klib.h"
#include "console.h"
#include "bootloader.h"
#include "pmm.h"
#include "serial.h"
#include "panic.h"
#include "forth/forth.h"
#include "symbols.h"
#include "x86_64/cpu.h"
#include "x86_64/mmu.h"
#include "x86_64/time.h"
#include "x86_64/descriptors.h"
#include "x86_64/cpu.h"
#include "tree.h"
#include "memory.h"

static struct Outputs {
    bool serial, console;
} outputs;

void write(const char* data, isize length) {
    if (outputs.console) for (isize i=0; i<length; i++) console_write(data[i]);
    if (outputs.serial) for (isize i=0; i<length; i++) serial_write(data[i]);
}

int putchar(int c)
{
    char ch = c;
    write(&ch, 1);
    return c;
}

int puts(const char* s)
{
    write(s, strlen(s));
    putchar('\n');
    return 0;
}

void
klog(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    auto time = timestamp();
    printf("[ %6zi.%06zu ] ", time.seconds, time.nanoseconds / 1000);
    vprintf(fmt, args);
    va_end(args);
}

const char* lines[] = {
    "1 2 3 swap",
    "1 : x if 2 else 3 then ; false x true x",
    ": x 0 3 - begin dup while dup . 1+ repeat drop ; x",
};

static int runcode(const char* code, int length) {
    auto p = mmu_get_address_space();
    auto x = mmu_create_address_space();
    mmu_set_address_space(x);
    mmu_assign(x, 0x20000, ROUND_UP_P2(length, PAGE_SIZE), MMU_MODE_EXEC|MMU_MODE_WRITE, MMU_CACHE_DEFAULT);
    memcpy((void*)0x20000, code, length);
    int r = ((int(*)(void))0x20000)(); // <-- Scariest line of C ever
    mmu_set_address_space(p);
    mmu_destroy_address_space(x);
    return r;
}

void entry(void* stack) {
    symbol_table_init();

    int port = serial_init();
    if (port) {
        outputs.serial = true;
        klog("entry: serial port @%x initialised\n", port);
    }

    bootloader_init_display();
    outputs.console = true;
    klog("entry: display initialised\n");

    tsc_init();

    klog("entry: boot stack %p\n", stack);

    {
        puts("~~~ Symbols ~~~");
        int local = 0, global = 0;
        for (const struct Symbol* s = symbol_list; s; s = s->next) {
            if (isupper(s->type)) {
                //printf("%p\t%c\t%s\n", s->address, s->type, s->name);
                printf("%s ", s->name);
                global++;
            } else {
                local++;
            }
        }
        putchar('\n');
        klog("entry: total symbols: global %i local %i\n", global, local);
    }

    forth_init();

    {
        int words = 0;
        puts("\n~~~ Forth Words ~~~");
        for (const struct Forth_header* f = forth_headers; f; f = f->next) {
            //printf("%p\t%p\t%i\t%i\t%s\n", f, forth_header_to_xt(f), f->immediate, f->name_length, f->name);
            printf("%s ", f->name);
            words++;
        }
        putchar('\n');
        klog("entry: total forth words: %i\n", words);
    }

    static isize fstack[16];
    for (int i=0; i<ARRAY_LENGTH(lines); i++) {
        forth_interpret(lines[i], strlen(lines[i]), fstack+1);
    }

    x86_64_load_early_descriptors();

    bootloader_run_setup();

    x86_64_load_descriptors((usize)stack);
    x86_64_cpu_create_tls(0, (usize)stack);

    alloc_print_info();
    pmm_print_info();

    printf("%i\n", runcode("\xB8\x0C\x00\x00\x00\xC3", 6));
    pmm_print_info();
    runcode("\xC7\x04\x25\x34\x12\x00\x00\x07\x00\x00\x00", 10);

    printf("entry @ %zx\n", virt_to_phys(mmu_get_address_space(), &entry).address);

    panic("reached end of main");
}
