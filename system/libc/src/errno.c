#include <errno.h>

thread_local int __errno;

int*
__errno_location()
{
    return &__errno;
}
