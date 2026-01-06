#include "mem/alloc.h"
#include "mem/pmm.h"

#include "print/dest.h"
#include "print/serial.h"

#include "klib.h"
#include "bootloader.h"
#include "symbols.h"

#include "user/process.h"
#include "user/schedule.h"
#include "x86_64/cpu.h"
#include "x86_64/time.h"
#include "x86_64/descriptors.h"
#include "x86_64/cpu.h"

#include "forth/forth.h"

const char* lines[] = {
    "1 2 3 swap",
    "1 : x if 2 else 3 then ; false x true x",
    ": x 0 3 - begin dup while dup . 1+ repeat drop ; x",
};

void entry(void* stack) {
    symbol_table_init();

    int port = serial_init();
    if (port) {
        print_dest_enable(PRINT_DEST_SERIAL);
        klog("entry: serial port @%x initialised\n", port);
    }

    bootloader_init_display();
    print_dest_enable(PRINT_DEST_CONSOLE);
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

    static const char program[] =
        "\x90\x0f\x05"
        "\x90\xB8\x01\x00\x00\x00\xFF\xC2\xC7\x04\x25\x34\x12\x00\x00\x0C\x00\x00\x00";

    auto proc = process_create();
    process_load_flat_binary(proc, program, sizeof(program));
    schedule_ready(proc);
    schedule();

    panic("reached end of main");
}
