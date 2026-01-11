#include "descriptor.h"
#include "klib.h"
#include "mem/alloc.h"
#include "errno.h"
#include "handle.h"

constexpr int L0 = ARRAY_LENGTH(((struct Descriptor_table*)0)->l0);
constexpr int L1 = ARRAY_LENGTH(((struct Descriptor_table*)0)->l1);
constexpr int L1L0 = ARRAY_LENGTH(((struct Descriptor_table*)0)->l1[0]->l0);

constexpr int DIRECT = L0;
constexpr int INDIRECT = L1 * L1L0;

struct Descriptor*
descriptor_new(struct Descriptor_table* table, int* fd)
{
    int n = 0;

    // Direct entries
    for (int i=0; i<L0; i++, n++) {
        if (!table->l0[i].open) {
            *fd = n;
            return &table->l0[i];
        }
    }

    // Indirect entries
    for (int i=0; i<L1; i++) {
        auto l1 = table->l1[i];

        if (!l1) {
            l1 = kalloc(sizeof(*l1));
            memset(l1, 0, sizeof(*l1));
            if (!l1)
                return nullptr;
            table->l1[i] = l1;
            *fd = n;
            return &l1->l0[0];
        }

        for (int j=0; j<L1L0; j++, n++) {
            if (!l1->l0[j].open) {
                *fd = n;
                return &l1->l0[j];
            }
        }
    }

    errno = EMFILE;
    return nullptr;
}

struct Descriptor*
descriptor_get(struct Descriptor_table* table, int fd)
{
    if (fd < 0)
        goto fail;

    if (fd < DIRECT) {
        auto l0 = &table->l0[fd];
        if (!l0->open)
            goto fail;
        return l0;
    }

    fd -= DIRECT;
    if (fd < INDIRECT) {
        auto l1 = table->l1[fd / L1L0];
        if (!l1)
            goto fail;
        auto l0 = &l1->l0[fd % L1L0];
        if (!l0->open)
            goto fail;
        return l0;
    }
fail:
    errno = EBADF;
    return nullptr;
}

void
descriptor_assign(struct Descriptor* descriptor, struct Handle* handle)
{
    assert(!descriptor->open);
    assert(handle->refcount >= 0);
    *descriptor = (struct Descriptor) { .open = true, .handle = handle };
    handle->refcount++;
}
