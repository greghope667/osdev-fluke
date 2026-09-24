#include <string.h>

size_t
strnlen(const char* s, size_t maxlen)
{
    auto found = memchr(s, 0, maxlen);
    return found ? (const char*)found - s : maxlen;
}
