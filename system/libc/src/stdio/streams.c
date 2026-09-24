#include "file.h"
#include "libc_impl.h"
#include <errno.h>
#include <fcntl.h>

constexpr int N = 16;

static FILE streams[N];

FILE* stdin;
FILE* stdout;
FILE* stderr;

FILE*
__libc_alloc_stream()
{
    for (size_t i=0; i<N; i++) {
        FILE* f = streams + i;
        if (!is_open(f))
            return f;
    }
    errno = EMFILE;
    return nullptr;
}

void
__libc_init_stdio()
{
    stdin = __libc_fdopen(0, O_RDONLY);
    stdout = __libc_fdopen(1, O_WRONLY);
    stderr = __libc_fdopen(2, O_WRONLY);
    // TODO: assert all open
}
