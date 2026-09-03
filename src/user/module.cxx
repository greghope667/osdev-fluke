#include "handle.hxx"
#include "mem/alloc.hxx"
#include "mem/memory.h"
#include "fluke/seek.h"

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
    result<isize> seek(isize offset, int whence) override;
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

result<isize>
Module::seek(isize offset, int whence)
{
    switch (whence) {
    case SEEK_SET:
        break;
    case SEEK_CUR:
        offset += this->offset;
        break;
    case SEEK_END:
        offset += this->size;
        break;
    default:
        return error_code(EINVAL);
    }
    if (offset < 0 || offset > this->size)
        return error_code(EINVAL);
    this->offset = offset;
    return offset;
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
