#include "dest.h"
#include "console.h"
#include "serial.h"
#include "klib.h"
#include "x86_64/time.h"

static enum print_dest stdout_dest = 0;

void print_dest_enable(enum print_dest dest)
{
    stdout_dest |= dest;
}

void print_dest_disable(enum print_dest dest)
{
    stdout_dest &= ~dest;
}

void write(const char* data, isize length) {
    if (stdout_dest & PRINT_DEST_CONSOLE)
        for (isize i=0; i<length; i++) console_write(data[i]);
    if (stdout_dest & PRINT_DEST_SERIAL)
        for (isize i=0; i<length; i++) serial_write(data[i]);
}

int putchar(int c)
{
    if (stdout_dest & PRINT_DEST_CONSOLE)
        console_write(c);
    if (stdout_dest & PRINT_DEST_SERIAL)
        serial_write(c);
    return c;
}

int puts(const char* s)
{
    write(s, strlen(s));
    putchar('\n');
    return 0;
}

void
klog(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    auto time = timestamp();
    printf("[ %6zi.%06zu ] ", time.seconds, time.nanoseconds / 1000);
    vprintf(fmt, args);
    va_end(args);
}
