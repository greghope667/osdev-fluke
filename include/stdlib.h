#pragma once
// https://pubs.opengroup.org/onlinepubs/9799919799/basedefs/stdlib.h.html

#include <fluke/types.h>

#include <stddef.h>                     // IWYU pragma: export

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

#ifdef __cplusplus
extern "C" {
#endif

void    _Exit(int __status) __NOTHROW __NORETURN;
void    abort(void) __NOTHROW __NORETURN;
int     abs(int) __NOTHROW;
int     atexit(void (*__func)(void)) __NOTHROW;
int     atoi(const char* __str) __NOTHROW;
void*   calloc(size_t __nitems, size_t __size) __NOTHROW;
void    exit(int __status) __NOTHROW __NORETURN;
void    free(void*) __NOTHROW;
char*   getenv(const char* __key) __NOTHROW;
void*   malloc(size_t __bytes) __NOTHROW;

#ifdef __cplusplus
} // extern C
#endif
