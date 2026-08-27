#include "handle.hxx"
#include "mem/alloc.hxx"
#include "mem/memory.h"

struct Module : Handle {
    const char* address;
    isize size;
    isize offset;

    Module(const void* data, isize size)
        : address((const char*)data)
        , size(size)
    {
        assert(size > 0);
    }

    result<isize> read(void* buffer, isize len) override;
    void close() final;
};

result<isize>
Module::read(void* dest, isize len)
{
    assert(len > 0);
    isize bytes = MIN(len, size - offset);
    TRY_ERRC(copy_to_user(dest, address + offset, bytes));
    offset += bytes;
    return bytes;
}

extern "C"
Handle*
handle_open_module(const void* data, isize size)
{
    assert(size > 0);
    auto m = kalloc_t<Module>();
    if (not m) return nullptr;
    return new(m) Module{data, size};
}

void
Module::close()
{
    kfree_t(this);
}
