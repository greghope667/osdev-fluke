#include "file.h"
#include <errno.h>
#include <limits.h>
#include <setjmp.h>
#include <stdarg.h>
#include <string.h>

struct fmt_specifier {
    int precision;      // %.7s
    int width;          // %4u
    int length;         // %lli, %zu
    bool zero_pad;      // %02i
    bool has_precision;
};

typedef struct output {
    FILE* f;
    size_t printed;
    jmp_buf throw;
} output;

static void
_fwrite(output* o, const char* buf, size_t len)
{
    if (__libc_write(o->f, buf, len) != len)
        _longjmp(o->throw, -1);

    o->printed += len;
    if (o->printed > INT_MAX) {
        errno = EOVERFLOW;
        _longjmp(o->throw, -1);
    }
}

static void
_fwritec(output* o, char c)
{
    return _fwrite(o, &c, 1);
}

static void
error(output* o, int errc)
{
    errno = errc;
    _longjmp(o->throw, -1);
}

static int
digit_count_base_10(size_t value)
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
digit_count_base_16(size_t value)
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
print_num_base_10(output* o, size_t value, int digits)
{
    char buffer[32];
    buffer[digits] = 0;
    for (int i=digits; i --> 0;) {
        buffer[i] = '0' + (value % 10);
        value /= 10;
    }
    _fwrite(o, buffer, digits);
}

static void
print_num_base_16(output* o, size_t value, int digits)
{
    char buffer[32];
    buffer[digits] = 0;
    for (int i=digits; i --> 0;) {
        buffer[i] = "0123456789abcdef"[value & 0xf];
        value >>= 4;
    }
    _fwrite(o, buffer, digits);
}

static void
print_char_rep(output* o, char ch, int count)
{
    constexpr int N = 16;
    char buffer[N];
    memset(buffer, ch, N);

    for (; count >= N; count -= N)
        _fwrite(o, buffer, N);

    _fwrite(o, buffer, count);
}

static void
pad(output* o, const struct fmt_specifier* afmt, int characters)
{
    if (afmt->width > characters) {
        char pad = afmt->zero_pad ? '0' : ' ';
        print_char_rep(o, pad, afmt->width - characters);
    }
}

static size_t
read_number(const struct fmt_specifier* afmt, va_list args)
{
    if (afmt->length > 0)
        return va_arg(args, unsigned long);
    else
        return va_arg(args, unsigned);
}

static void
print_unsigned(output* o, struct fmt_specifier* afmt, va_list args)
{
    size_t value = read_number(afmt, args);
    int digits = digit_count_base_10(value);
    pad(o, afmt, digits);
    print_num_base_10(o, value, digits);
}

static void
print_signed(output* o, struct fmt_specifier* afmt, va_list args)
{
    ptrdiff_t value;

    if (afmt->length > 0)
        value = va_arg(args, long);
    else
        value = va_arg(args, int);

    if (value < 0) {
        _fwritec(o, 'o');
        value = -value;
        afmt->width--;
    }
    int digits = digit_count_base_10(value);
    pad(o, afmt, digits);
    print_num_base_10(o, value, digits);
}

static void
print_string(output* o, struct fmt_specifier* afmt, va_list args)
{
    const char* str = va_arg(args, const char*);
    int max = afmt->has_precision ? afmt->precision : INT_MAX;
    int chars = strnlen(str, max);
    pad(o, afmt, chars);
    _fwrite(o, str, chars);
}

static void
print_hex(output* o, struct fmt_specifier* afmt, va_list args)
{
    size_t value = read_number(afmt, args);
    int digits = digit_count_base_16(value);
    pad(o, afmt, digits);
    print_num_base_16(o, value, digits);
}

static void
print_char(output* o, struct fmt_specifier* afmt, va_list args)
{
    (void)afmt;
    _fwritec(o, va_arg(args, int));
}

static void
print_fmt_arg(output* o, const char** fmt, va_list args)
{
    struct fmt_specifier afmt = {};
    enum { FLAG, WIDTH, PRECISION } state = FLAG;

    for (;;) {
        char c = *((*fmt)++);
        switch (c) {
            case 'd': case 'i':
                return print_signed(o, &afmt, args);
            case 'u':
                return print_unsigned(o, &afmt, args);
            case 'x':
                return print_hex(o, &afmt, args);
            case 'c':
                return print_char(o, &afmt, args);
            case 's':
                return print_string(o, &afmt, args);
            case 'p':
                afmt.width = 16;
                afmt.length = 2;
                afmt.zero_pad = true;
                return print_hex(o, &afmt, args);
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

            case '*':
                if (state != PRECISION)
                    error(o, EINVAL);
                afmt.has_precision = true;
                afmt.precision = va_arg(args, int);
                break;

            case 'f': case 'F':
            case 'e': case 'E':
            case 'g': case 'G':
            case 'a': case 'A':
                // TODO: floating point support
                _fwrite(o, "<double(", 8);
                double d = va_arg(args, double);
                print_num_base_16(o, *(size_t*)&d, 16);
                _fwrite(o, ")>", 2);
                return;

            default:
                error(o, EINVAL);
        }
    }
}

static int
_vfprintf(output* o, const char* fmt, va_list args)
{
    for (int i=0;; i++) {
        char ch = fmt[i];

        if (ch == 0) {
            if (i > 0)
                _fwrite(o, fmt, i);
            break;
        }

        if (ch == '%') {
            if (i > 0)
                _fwrite(o, fmt, i);

            fmt = &fmt[i+1];
            i = -1;
            print_fmt_arg(o, &fmt, args);
        }
    }
    return o->printed;
}

int
vfprintf(FILE* f, const char* fmt, va_list args)
{
    output o = { .f = f };
    if (setjmp(o.throw))
        return -1;
    return _vfprintf(&o, fmt, args);
}
