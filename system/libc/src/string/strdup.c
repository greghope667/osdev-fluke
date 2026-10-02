#include <string.h>
#include <stdlib.h>

char*
strdup(const char* str)
{
    auto len = strlen(str);
    char* ptr = malloc(len + 1);
    if (!ptr)
        return nullptr;
    ptr[len] = 0;
    return memcpy(ptr, str, len);
}
