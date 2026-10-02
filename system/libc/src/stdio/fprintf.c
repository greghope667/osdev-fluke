#include <stdio.h>
#include <stdarg.h>

int
fprintf(FILE* f, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vfprintf(f, fmt, args);
    va_end(args);
    return 0;
}
