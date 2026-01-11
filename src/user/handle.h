#pragma once

#include "kdef.h"

struct Handle_vtbl;

struct Handle {
    const struct Handle_vtbl* vtbl;
    isize refcount;
    // ...
};

struct Handle_vtbl {
    void (*destruct)(struct Handle*);
    isize (*write)(struct Handle*, const void* data, isize len);
    isize (*read)(struct Handle*, void* data, isize len);
};

void handle_release(struct Handle*);
