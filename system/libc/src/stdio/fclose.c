#include "file.h"
#include <string.h>

int
fclose(FILE* f)
{
    int r = f->closefn(f->cookie);
    memset(f, 0, sizeof(*f));
    return r >= 0 ? 0 : -1;
}
