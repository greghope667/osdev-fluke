#include "symbols.h"

#include "klib.h"

const struct Symbol* symbol_list = 0;

void symbol_table_init()
{
    INIT_ONCE

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
        sym->length = MIN(len, MAX_SYMBOL_LENGTH);

        symbols_text = &name[len+1];
    }

    sym->next = 0;
    symbol_list = head.next;
}

const struct Symbol*
ksym_s(const char* name)
{
    isize len = strnlen(name, MAX_SYMBOL_LENGTH);
    for (auto s = symbol_list; s; s = s->next) {
        if (s->length == len && isupper(s->type) && memcmp(s->name, name, len) == 0)
            return s;
    }
    return nullptr;
}

const struct Symbol*
ksym_n(const char* name, isize len)
{
    len = MIN(len, MAX_SYMBOL_LENGTH);
    for (auto s = symbol_list; s; s = s->next) {
        if (s->length == len && isupper(s->type) && memcmp(s->name, name, len) == 0)
            return s;
    }
    return nullptr;
}
