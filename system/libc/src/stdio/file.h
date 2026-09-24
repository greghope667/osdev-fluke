#include "libc_impl.h"
#include <stdio.h>

struct FILE {
    union {
        long fd;
        const void* cookie;
    };
    bool error, eof, is_cookie;
    int (*writefn)(void*, const char*, int);
    int (*readfn)(void*, char*, int);
    off_t (*seekfn)(void*, off_t, int);
    int (*closefn)(void*);
};

PRIVATE size_t __libc_write(FILE* f, const void* data, size_t size);
PRIVATE size_t __libc_read(FILE* f, void* data, size_t size);
