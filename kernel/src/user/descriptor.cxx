#include "descriptor.hxx"
#include "klib.h"
#include "mem/alloc.hxx"
#include "handle.hxx"

void
Descriptor::assign(Handle* h, bool cloexec)
{
    assert(not is_open());
    assert(h->refcount() >= 0);
    h->dup();
    _handle = (Handle*)((usize)h | (usize)cloexec);
}

void
Descriptor::close()
{
    assert(is_open());
    handle()->release();
    _handle = nullptr;
}

constexpr int DIRECT = Descriptor_table::L0;
constexpr int INDIRECT = Descriptor_table::L1 * Descriptor_table::L1L0;

result<Descriptor*>
Descriptor_table::alloc(int& fd)
{
    int n = 0;

    // Direct entries
    for (auto& desc : l0) {
        if (not desc.is_open()) {
            fd = n;
            return &desc;
        }
        n++;
    }

    // Indirect entries
    for (auto& l0 : l1) {
        if (not l0) {
            l0 = TRY(owned<table_l1l0>::make());
            fd = n;
            return &(*l0)[0];
        }

        for (auto& desc : *l0) {
            if (not desc.is_open()) {
                fd = n;
                return &desc;
            }
            n++;
        }
    }
    return error_code(EMFILE);
}

result<Descriptor*>
Descriptor_table::alloc_overwrite(int fd)
{
    auto fail = error_code(EBADF);

    if (fd < 0)
        return fail;

    if (fd < DIRECT) {
        auto& desc = l0[fd];
        if (desc.is_open())
            desc.close();
        return &desc;
    }

    fd -= DIRECT;
    if (fd < INDIRECT) {
        auto& l0 = l1[fd / L1L0];
        if (!l0)
            l0 = TRY(owned<table_l1l0>::make());
        auto& desc = (*l0)[fd % L1L0];
        if (desc.is_open())
            desc.close();
        return &desc;
    }

    return fail;
}

result<Descriptor*>
Descriptor_table::get(int fd)
{
    auto fail = error_code(EBADF);

    if (fd < 0)
        return fail;

    if (fd < DIRECT) {
        auto& desc = l0[fd];
        if (not desc.is_open())
            return fail;
        return &desc;
    }

    fd -= DIRECT;
    if (fd < INDIRECT) {
        auto& l0 = l1[fd / L1L0];
        if (!l0)
            return fail;
        auto& desc = (*l0)[fd % L1L0];
        if (not desc.is_open())
            return fail;
        return &desc;
    }

    return fail;
}

result<void>
Descriptor_table::close(int fd)
{
    TRY(get(fd))->close();
    return {};
}

template <size_t N>
static void
clone(std::array<Descriptor, N>& from, std::array<Descriptor, N>& to)
{
    for (size_t i=0; i<N; i++) {
        assert(not to[i].is_open());
        if (from[i].is_open()) {
            to[i].assign(from[i].handle(), from[i].is_cloexec());
        }
    }
}

result<void>
Descriptor_table::clone(Descriptor_table& to)
{
    ::clone(l0, to.l0);
    for (int i=0; i<L1; i++) {
        assert(not to.l1[i]);
        if (l1[i]) {
            to.l1[i] = TRY(owned<table_l1l0>::make());
            ::clone(*l1[i], *to.l1[i]);
        }
    }
    return {};
}

void
Descriptor_table::cloexec()
{
    for (auto& desc : l0) {
        if (desc.is_cloexec())
            desc.close();
    }
    for (auto& l0 : l1) {
        if (not l0)
            continue;

        for (auto& desc : *l0) {
            if (desc.is_cloexec())
                desc.close();
        }
    }
}
