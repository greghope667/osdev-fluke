#pragma once
// https://pubs.opengroup.org/onlinepubs/9799919799/basedefs/fcntl.h.html

#include <fluke/types.h>

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

// oflags

#define O_EXEC          (1 << 0)
#define O_RDONLY        (1 << 1)
#define O_RDWR          (1 << 2)
#define O_SEARCH        (1 << 3)
#define O_WRONLY        (1 << 4)

#define O_APPEND        (1 << 5)
#define O_CLOEXEC       (1 << 6)
#define O_CLOFORK       (1 << 7)
#define O_CREAT         (1 << 8)
#define O_DIRECTORY     (1 << 9)
#define O_EXCL          (1 << 10)
#define O_NOCTTY        (1 << 11)
#define O_NOFOLLOW      (1 << 12)
#define O_NONBLOCK      (1 << 13)
#define O_SYNC          (1 << 14)
#define O_TRUNC         (1 << 15)


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
