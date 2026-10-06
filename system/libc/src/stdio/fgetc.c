#include "file.h"

static int
_fgetc(FILE* f)
{
    unsigned char c;
    return __libc_read(f, &c, 1) ? c : EOF;
}

int
fgetc(FILE* f) __attribute__((alias("_fgetc")));

int
getc(FILE* f) __attribute__((alias("_fgetc")));

int
getchar()
{
    return _fgetc(stdin);
}
