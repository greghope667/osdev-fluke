#include "file.h"

extern int __io_putchar(int ch);

static int
stdout_write(void*, const char* s, int n)
{
    for (int i=0; i<n; i++) {
        if (__io_putchar(s[i]) == EOF)
            return -1;
    }
    return n;
}

FILE stdout_s = {
    .fd = 0,
    .writefn = stdout_write,
};

FILE* stdout = &stdout_s;
