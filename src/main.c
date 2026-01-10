#include "print/dest.h"
#include "print/serial.h"

#include "klib.h"
#include "bootloader.h"
#include "symbols.h"

#include "user/process.h"
#include "user/schedule.h"
#include "x86_64/apic.h"
#include "x86_64/cpu.h"
#include "x86_64/time.h"
#include "x86_64/descriptors.h"
#include "x86_64/cpu.h"

#include "forth/forth.h"

extern const char pid0_code[];
extern const usize pid0_size;

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
        int local = 0, global = 0;
        for (const struct Symbol* s = symbol_list; s; s = s->next)
            isupper(s->type) ? global++ : local++;
        klog("entry: total symbols: global %i local %i\n", global, local);
    }

    forth_init();
    {
        int words = 0;
        for (const struct Forth_header* f = forth_headers; f; f = f->next)
            words++;
        klog("entry: total forth words: %i\n", words);
    }

    x86_64_load_early_descriptors();

    bootloader_run_setup();

    x86_64_load_descriptors((usize)stack);
    x86_64_cpu_create_tls(0, (usize)stack);

    x86_64_apic_initialise();
    x86_64_ioapic_initialise();

    // alloc_print_info();
    // pmm_print_info();

    for (int i=0; i<2; i++) {
        auto proc = process_create();
        process_load_flat_binary(proc, pid0_code, pid0_size);
        schedule_ready(proc);
    }
    x86_64_apic_set_tickrate(1);
    schedule();
    // cpu_exit_idle();

    panic("reached end of main");
}
