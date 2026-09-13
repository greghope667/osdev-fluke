#include <stdlib.h>
#include "libc_impl.h"

typedef void (*atexit_fptr)(void);

static atexit_fptr atexit_array[64] = {};

int
atexit(atexit_fptr p)
{
    if (!p)
        return 0;

    for (int i=0; i<64; i++) {
        if (!atexit_array[i]) {
            atexit_array[i] = p;
            return 0;
        }
    }

    return -1;
}

void
__libc_do_atexit()
{
    for (int i=64; i --> 0;) {
        if (atexit_array[i])
            atexit_array[i]();
    }
}
