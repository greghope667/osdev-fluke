#include <string.h>

int
strcmp(const char* a, const char* b)
{
    while (*a++ == *b++) {
        if (a[-1] == 0)
            return 0;
    }
    return a[-1] > b[-1] ? 1 : -1;
}
