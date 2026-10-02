#include <string.h>

int
memcmp(const void* ap, const void* bp, size_t len)
{
    const char* a = ap;
    const char* b = bp;
    while (len --> 0)
        if (*a++ != *b++)
            return a[-1] > b[-1] ? 1 : -1;
    return 0;
}
