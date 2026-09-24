#include "file.h"

static int
_fputc(int ch, FILE* f)
{
    unsigned char c = ch;
    return __libc_write(f, &c, 1) ? c : EOF;
}

int
fputc(int ch, FILE* f) __attribute__((alias("_fputc")));

int
putc(int ch, FILE* f) __attribute__((alias("_fputc")));

int
putchar(int ch)
{
    return _fputc(ch, stdout);
}
