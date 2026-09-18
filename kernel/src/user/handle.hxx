#pragma once

#include "klib.hxx"

struct Registers;

struct Handle {
    virtual result<isize> read(void* buffer, isize len);
    virtual result<isize> write(const void* data, isize len);
    virtual result<isize> seek(isize offset, int whence);

    virtual result<usize> ctl(Registers* ctx, unsigned op);
    virtual error_code ipc_call(Registers* ctx);
    // virtual ~Handle();

    void dup() { ref_count_++; };
    isize refcount() { return ref_count_; }
    void release();

private:
    virtual void close() = 0;
    isize ref_count_ = 0;
};
