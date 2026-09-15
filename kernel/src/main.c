#include "print/dest.h"
#include "print/serial.h"

#include "klib.h"
#include "bootloader.h"
#include "symbols.h"

#include "user/init.h"
#include "user/schedule.h"
#include "x86_64/apic.h"
#include "x86_64/cpu.h"
#include "x86_64/mmu.h"
#include "x86_64/time.h"
#include "x86_64/descriptors.h"
#include "x86_64/tls.h"

#include "forth/forth.h"

#include "share/share.h"

void _main(void* stack) {
    symbol_table_init();

    int port = serial_init();
    if (port) {
        print_dest_enable(PRINT_DEST_SERIAL);
        klog("entry: serial port @%x initialised\n", port);
    }

    bootloader_init_display();
    print_dest_enable(PRINT_DEST_CONSOLE);
    klog("entry: display initialised\n");

    x86_64_tsc_init();

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
    x86_64_fpu_initialise();

    mmu_configure_root_address_space();
    user_share_init();
    user_init();

    x86_64_apic_initialise();
    x86_64_ioapic_initialise();

    x86_64_apic_set_tickrate(1);
    schedule();

    panic("reached end of main");
}

void
__cxa_pure_virtual()
{
    panic(__FUNCTION__);
}
