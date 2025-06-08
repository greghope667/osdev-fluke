#include "main.h"
#include "panic.h"
#include "types.h"

#include <stdarg.h>

struct fmt_specifier {
    int precision;      // %.7s
    int width;          // %4u
    int length;         // %lli, %zu
    bool zero_pad;      // %02i
    bool has_precision;
};

static int
digit_count_base_10(u64 value)
{
    if (value == 0)
        return 1;

    int count = 0;
    while (value > 0) {
        value /= 10;
        count++;
    }
    return count;
}

static int
digit_count_base_16(u64 value)
{
    if (value == 0)
        return 1;

    int count = 0;
    while (value > 0) {
        value /= 16;
        count++;
    }
    return count;
}

static void
print_num_base_10(u64 value, int digits)
{
    char buffer[32];
    buffer[digits] = 0;
    for (int i=digits; i --> 0;) {
        buffer[i] = '0' + (value % 10);
        value /= 10;
    }
    write(buffer, digits);
}

static void
print_num_base_16(u64 value, int digits)
{
    char buffer[32];
    buffer[digits] = 0;
    for (int i=digits; i --> 0;) {
        buffer[i] = "0123456789abcdef"[value & 0xf];
        value >>= 4;
    }
    write(buffer, digits);
}

static void
print_char_rep(char ch, int count)
{
    char buffer[32];
    memset(buffer, ch, 32);
    for (; count >= 32; count -= 32)
        write(buffer, 32);

    write(buffer, count);
}

static void
pad(const struct fmt_specifier* afmt, int characters)
{
    if (afmt->width > characters) {
        char pad = afmt->zero_pad ? '0' : ' ';
        print_char_rep(pad, afmt->width - characters);
    }
}

static usize
read_number(const struct fmt_specifier* afmt, va_list args)
{
    switch (afmt->length) {
        case -2: case -1: case 0:
            return va_arg(args, unsigned);
        case 1:
            return va_arg(args, unsigned long);
        case 2:
            return va_arg(args, unsigned long long);
        default:
            panic("invalid format string");
    }
}

static void
print_unsigned(struct fmt_specifier* afmt, va_list args)
{
    usize value = read_number(afmt, args);
    int digits = digit_count_base_10(value);
    pad(afmt, digits);
    print_num_base_10(value, digits);
}

static void
print_signed(struct fmt_specifier* afmt, va_list args)
{
    isize value;

    switch (afmt->length) {
        case -2: case -1: case 0:
            value = va_arg(args, int);
            break;
        case 1:
            value = va_arg(args, long);
            break;
        case 2:
            value = va_arg(args, long long);
            break;
        default:
            panic("invalid format string");
    }

    if (value < 0) {
        putchar('-');
        value = -value;
        afmt->width--;
    }
    int digits = digit_count_base_10(value);
    pad(afmt, digits);
    print_num_base_10(value, digits);
}

static void
print_string(struct fmt_specifier* afmt, va_list args)
{
    const char* str = va_arg(args, const char*);
    int max = afmt->has_precision ? afmt->precision : INT_MAX;
    int chars = strnlen(str, max);
    pad(afmt, chars);
    write(str, chars);
}

static void
print_hex(struct fmt_specifier* afmt, va_list args)
{
    usize value = read_number(afmt, args);
    int digits = digit_count_base_16(value);
    pad(afmt, digits);
    print_num_base_16(value, digits);
}

static void
print_char(struct fmt_specifier* afmt, va_list args)
{
    (void)afmt;
    putchar(va_arg(args, int));
}

static void
print_fmt_arg(const char** fmt, va_list args)
{
    struct fmt_specifier afmt = {};
    enum { FLAG, WIDTH, PRECISION } state = FLAG;

    for (;;) {
        char c = *((*fmt)++);
        switch (c) {
            case 'd': case 'i':
                return print_signed(&afmt, args);
            case 'u':
                return print_unsigned(&afmt, args);
            case 'x':
                return print_hex(&afmt, args);
            case 'c':
                return print_char(&afmt, args);
            case 's':
                return print_string(&afmt, args);
            case 'p':
                afmt.width = 16;
                afmt.length = 2;
                afmt.zero_pad = true;
                return print_hex(&afmt, args);
            case '%':
                putchar('%');
                return;

            case 'h':
                afmt.length--;
                break;
            case 'l':
                afmt.length++;
                break;

            case 'z':
                afmt.length = 1;
                break;

            case '0':
                switch (state) {
                    case FLAG:
                        afmt.zero_pad = true;
                        state = WIDTH;
                        break;
                    case WIDTH:
                        afmt.width *= 10;
                        break;
                    case PRECISION:
                        afmt.precision *= 10;
                        break;
                }
                break;

            case '1'...'9':
                switch (state) {
                    case FLAG:
                        state = WIDTH;
                        [[fallthrough]];
                    case WIDTH:
                        afmt.width = afmt.width * 10 + (c - '0');
                        break;
                    case PRECISION:
                        afmt.precision = afmt.precision * 10 + (c - '0');
                        break;
                }
                break;

            case '.':
                state = PRECISION;
                afmt.has_precision = true;
                break;

            default:
                panic("unsupported printf specifier");
        }
    }
}

int
vprintf(const char* fmt, va_list args)
{
    for (int i=0;; i++) {
        char ch = fmt[i];

        if (ch == 0) {
            if (i > 0)
                write(fmt, i);
            break;
        }

        if (ch == '%') {
            if (i > 0)
                write(fmt, i);

            fmt = &fmt[i+1];
            i = -1;
            print_fmt_arg(&fmt, args);
        }
    }
    return 0;
}

int
printf(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    return 0;
}

