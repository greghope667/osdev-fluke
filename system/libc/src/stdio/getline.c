#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include "file.h"

ssize_t
getdelim(char** restrict _lineptr, size_t* restrict _n, int delim, FILE* f)
{
    if (!_lineptr || !_n) {
        errno = EINVAL;
        return -1;
    }

    char* line = *_lineptr;
    size_t len = 0;
    size_t cap = line ? *_n : 0;

    for (;;) {
        if (len+1 >= cap) {
            cap = cap < 16 ? 16 : (cap * 3) / 2;
            line = realloc(line, cap);
            if (!line)
                return -1;
            *_lineptr = line;
            *_n = cap;
        }

        int ch = fgetc(f);

        if (ch == EOF) {
            if (f->error)
                return -1;
            break;
        }

        line[len++] = ch;
        if ((unsigned char)ch == (unsigned char)delim)
            break;
    }

    assert(len < cap);
    line[len] = 0;

    return len > 0 ? len : -1;
}

ssize_t
getline(char** lineptr, size_t* n, FILE* f)
{
    return getdelim(lineptr, n, '\n', f);
}
