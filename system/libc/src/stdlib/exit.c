#include <stdlib.h>
#include "libc_impl.h"

__NORETURN
void
exit(int status)
{
    __libc_do_atexit();
    _Exit(status);
}
