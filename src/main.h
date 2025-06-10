#pragma once

#include "types.h"

// Stuff that doesn't yet have a home

void* memcpy(void*, const void*, usize);
int memcmp(const void*, const void*, usize);
void* memchr(const void* p, int ch, usize n);
void* memset(void*, int, usize);
usize strnlen(const char*, usize);
usize strlen(const char*);

int putchar(int);
int puts(const char*);
int printf(const char*, ...) __attribute__((format(printf, 1, 2)));

#define isupper(ch) ({ auto _ch = (ch); (_ch >= 'A') && (_ch <= 'Z'); })
#define isprint(ch) ({ auto _ch = (ch); (_ch >= ' ') && (_ch <= 127); })

void write(const char*, isize);

struct Symbol {
    struct Symbol* next;
    void* address;
    char _space;
    char type;
    char length;
    char name[];
} __attribute__((packed));

void bootloader_init_display();

extern const struct Symbol* symbol_list;
