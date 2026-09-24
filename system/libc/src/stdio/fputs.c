#include "file.h"
#include <string.h>

static int
_fputs(const char* str, FILE* f)
{
    auto len = strlen(str);
    return __libc_write(f, str, len) == len ? 1 : EOF;
}

int
fputs(const char* str, FILE* f) __attribute__((alias("_fputs")));

int
puts(const char* str)
{
    return _fputs(str, stdout) > 0 ? putchar('\n') : EOF;
}
