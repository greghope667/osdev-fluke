#pragma once
// IWYU pragma: always_keep
#include "kdef.h" // IWYU pragma: export

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
#define isprint(ch) ({ auto _ch = (ch); (_ch >= ' ') && (_ch <= 126); })

/* Rounding to multiple of a given power of 2 */

#define ROUND_DOWN_P2(val, power2) ((val) & ~((power2) - 1))
#define ROUND_UP_P2(val, power2) (((val) + (power2) - 1) & ~((power2) - 1))

/* Panic handling (formerly panic.h) */

void panic(const char* reason) __attribute__((noreturn));
void show_backtrace(void* frame);
void show_backtrace_here();

#define assert(x) if (!(x)) panic("assertion failure")

#define INIT_ONCE {                         \
    static bool _done = false;              \
    if (_done) panic("INIT_ONCE check");    \
    _done = true;                           \
}
