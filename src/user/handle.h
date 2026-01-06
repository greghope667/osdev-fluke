#pragma once

#include "kdef.h"

#include "tree.h"

struct Handle_vtbl;

struct Handle {
    const struct Handle_vtbl* vtbl;
    struct Tree_node descriptors_node;
    // ...
};

struct Handle_vtbl {
    void (*destruct)(struct Handle*);
    isize (*write)(struct Handle*, const void* data, isize len);
    isize (*read)(struct Handle*, void* data, isize len);
};
