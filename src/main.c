#include "main.h"
#include "console.h"
#include "serial.h"
#include "types.h"
#include "panic.h"

const struct Symbol* symbol_list = 0;

void parse_symbol_table()
{
    static bool done = false;
    if (done) return;
    done = true;

    extern char __symbols[];
    char* symbols_text = (char*)__symbols;

    struct Symbol head = {};
    struct Symbol* sym = &head;

    while (*symbols_text != '\n') {
        unsigned long addr = 0;
        for (int i=0; i<16; i++) {
            char ch = symbols_text[i];
            addr <<= 4;
            addr += (ch > '9') ? (10 + ch - 'a') : (ch - '0');
        }

        char* name = &symbols_text[19];
        long len = (char*)memchr(name, '\n', PTRDIFF_MAX) - name;
        name[len] = 0;

        sym->next = (struct Symbol*)symbols_text;
        sym = sym->next;
        sym->address = (void*)addr;
        sym->length = MIN(len, 127);

        symbols_text = &name[len+1];
    }

    sym->next = 0;
    symbol_list = head.next;
}

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

void entry(void* stack) {
    parse_symbol_table();

    int port = serial_init();
    if (port) {
        outputs.serial = true;
        printf("serial port @%x started\n", port);
    }

    bootloader_init_display();
    outputs.console = true;
    printf("display initialised\n");

    printf("boot stack %p\n", stack);

    for (const struct Symbol* s = symbol_list; s; s = s->next) {
        printf("%p\t%c\t%s\n", s->address, s->type, s->name);
    }

    panic("reached end of main");

    (void)stack;
}
