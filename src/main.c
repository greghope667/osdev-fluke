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
#include "x86_64/time.h"
#include "x86_64/descriptors.h"
#include "x86_64/cpu.h"

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

    alloc_print_info();
    pmm_print_info();

    this_cpu->user_stack = 3;


    panic("reached end of main");
    for (;;) {
        asm volatile ("sti\nhlt");
    }


    (void)stack;
}
