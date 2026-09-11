#pragma once
// https://pubs.opengroup.org/onlinepubs/9799919799/basedefs/stdlib.h.html

#include <fluke/types.h>

#include <stddef.h>                     // IWYU pragma: export
#include <stdint.h>                     // IWYU pragma: export
#include <stdio.h>                      // IWYU pragma: export

typedef __size_t    size_t;
typedef __ssize_t   ssize_t;
typedef __off_t     off_t;
typedef __pid_t     pid_t;
typedef __intptr_t  intptr_t;

#ifdef __cplusplus
extern "C" {
#endif

void    _exit(int __status) __NOTHROW __NORETURN;
int     execv(const char* __path, char* const __argv[]) __NOTHROW;
int     execve(const char* __path, char* const __argv[], char* const __envp[]) __NOTHROW;
int     execvp(const char* __file, char* const __argv[], char* const __envp[]) __NOTHROW;
int     fexecve(int __fd, char* const __argv[], char* const __envp[]) __NOTHROW;
pid_t   fork() __NOTHROW;
pid_t   getpid() __NOTHROW;

#ifdef __cplusplus
} // extern C
#endif
