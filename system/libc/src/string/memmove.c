#include <string.h>

void*
memmove(void* dest, const void* src, size_t n)
{
    if (dest < src) {
        // Our memcpy copies forward
        return memcpy(dest, src, n);
    } else if (src + n <= dest) {
        // No overlap
        return memcpy(dest, src, n);
    }

    char* d = dest + n;
    const char* s = src + n;
    while (n --> 0)
        *--d = *--s;

    return dest;
}
