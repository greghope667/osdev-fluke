#include <string.h>
#include <stdlib.h>

char*
strndup(const char* str, size_t maxlen)
{
    auto len = strnlen(str, maxlen);
    char* ptr = malloc(len + 1);
    if (!ptr)
        return nullptr;
    ptr[len] = 0;
    return memcpy(ptr, str, len);
}
