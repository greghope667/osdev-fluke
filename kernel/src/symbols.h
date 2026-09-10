#pragma once

#include "kdef.h"

struct Symbol {
    struct Symbol* next;
    void* address;
    char _space;
    char type;
    char length;
    char name[];
} __attribute__((packed));

#define MAX_SYMBOL_LENGTH 127

extern const struct Symbol* symbol_list;

void symbol_table_init();

const struct Symbol* ksym_s(const char*);
const struct Symbol* ksym_n(const char*, isize);
const struct Symbol* symbol_of_address(void* address);
