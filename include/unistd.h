#pragma once
// https://pubs.opengroup.org/onlinepubs/9799919799/basedefs/stdlib.h.html

#include <fluke/types.h>

#include <fcntl.h>                      // IWYU pragma: export
#include <stddef.h>                     // IWYU pragma: export
#include <stdint.h>                     // IWYU pragma: export
#include <stdio.h>                      // IWYU pragma: export

typedef __size_t    size_t;
typedef __ssize_t   ssize_t;
typedef __off_t     off_t;
typedef __pid_t     pid_t;
typedef __intptr_t  intptr_t;

/*
// access() constants

#define F_OK        00      // existance
#define X_OK        01      // executable
#define W_OK        02      // writable
#define R_OK        04      // readable
*/

#ifdef __cplusplus
extern "C" {
#endif

void    _exit(int __status) __NOTHROW __NORETURN;
// int     access(const char* __path, int __access_mode);
int     close(int __fd);
int     dup(int __oldfd);
int     dup2(int __oldfd, int __newfd);
int     dup3(int __oldfd, int __newfd, int __flags);
int     execv(const char* __path, char* const __argv[]) __NOTHROW;
int     execve(const char* __path, char* const __argv[], char* const __envp[]) __NOTHROW;
int     execvp(const char* __file, char* const __argv[]) __NOTHROW;
int     fexecve(int __fd, char* const __argv[], char* const __envp[]) __NOTHROW;
pid_t   fork() __NOTHROW;
pid_t   getpid() __NOTHROW;
ssize_t read(int __fd, void* __restrict__ __buf, size_t __size) __NOTHROW;
ssize_t write(int __fd, const void* __restrict__ __buf, size_t __size) __NOTHROW;

#ifdef __cplusplus
} // extern C
#endif
