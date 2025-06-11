#pragma once

#include "kdef.h"

/* Common kernel functions, mostly libc functions (or variants thereof) */

/* String functions (currently these are defined in assembly) */

void* memcpy(void*, const void*, usize);
int memcmp(const void*, const void*, usize);
void* memchr(const void* p, int ch, usize n);
void* memset(void*, int, usize);
usize strnlen(const char*, usize);
usize strlen(const char*);

/* Basic IO */

int putchar(int);
int puts(const char*);
int printf(const char*, ...) __attribute__((format(printf, 1, 2)));
int vprintf(const char*, va_list args);
void klog(const char*, ...) __attribute__((format(printf, 1, 2)));

void write(const char*, isize);

/* Ctypes approximations */

#define isupper(ch) ({ auto _ch = (ch); (_ch >= 'A') && (_ch <= 'Z'); })
#define isprint(ch) ({ auto _ch = (ch); (_ch >= ' ') && (_ch <= 127); })
