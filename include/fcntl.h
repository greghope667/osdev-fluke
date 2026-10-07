#pragma once
// https://pubs.opengroup.org/onlinepubs/9799919799/basedefs/fcntl.h.html

#include <fluke/types.h>
#include <fluke/defs/fcntl.h>

typedef __mode_t        mode_t;
typedef __off_t         off_t;
typedef __pid_t         pid_t;

/*
struct flock {
    short   l_type;
    short   l_whence;
    off_t   l_start;
    off_t   l_len;
    off_t   l_pid;
};

// fcntl commands - record locking

#define F_GETLK             16
#define F_SETLK             17
#define F_SETLKW            18

#define F_RDLCK             1
#define F_UNLCK             2
#define F_WRLCK             3
*/

#ifdef __cplusplus
extern "C" {
#endif

int     creat(const char* __path, mode_t __mode) __NOTHROW;
// int     fcntl(int __fd, int __cmd, ...) __NOTHROW;
int     open(const char* __path, int __oflag, ...) __NOTHROW;
int     openat(int __dirfd, const char* __path, int __oflag, ...) __NOTHROW;

#ifdef __cplusplus
} //extern C
#endif
