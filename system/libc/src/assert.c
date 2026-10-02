#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

void
__assert_failed(const char* func, const char* file, int line)
{
    fprintf(stderr, "assert failed in %s() %s:%i\n", func, file, line);
    abort();
}
