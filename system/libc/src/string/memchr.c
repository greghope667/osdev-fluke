#include <string.h>

void*
memchr(const void* s, int ch, size_t n)
{
    const unsigned char* ptr = s;
    for (auto end = ptr + n; ptr != end; ptr++) {
        if (*ptr == (unsigned char)ch)
            return (void*)ptr;
    }
    return nullptr;
}
