#include "serial.h"
#include "types.h"

extern void* memchr(const void* p, int ch, unsigned long n);

struct Symbol {
    struct Symbol* next;
    void* address;
    char _space;
    char type;
    char length;
    char name[];
} __attribute__((packed));

const struct Symbol* symbol_list = 0;

void parse_symbol_table()
{
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
        sym->length = (len < 127) ? len : 127;

        symbols_text = &name[len+1];
    }

    sym->next = 0;
    symbol_list = head.next;
}

void putchar(int c)
{
    serial_write(c);
}

void puts(const char* s)
{
    while (*s)
        serial_write(*s++);
}

void putnumber(usize n)
{
    char buf[17] = {};
    for (int i=0; i<16; i++) {
        buf[15 - i] = "0123456789abcdef"[n & 0xf];
        n >>= 4;
    }
    puts(buf);
}

void entry(void* stack) {
    parse_symbol_table();
    serial_detect();

    for (const struct Symbol* s = symbol_list; s; s = s->next) {
        putnumber((usize)s->address);
        putchar('\t');
        putchar(s->type);
        putchar('\t');
        puts(s->name);
        putchar('\n');
    }

    (void)stack;
}
