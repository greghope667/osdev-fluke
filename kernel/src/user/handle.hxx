#pragma once

#include "klib.hxx"
#include "x86_64/cpu.h"

struct Handle {
    virtual result<isize> read(void* buffer, isize len);
    virtual result<isize> write(const void* data, isize len);
    virtual result<isize> seek(isize offset, int whence);

    virtual result<usize> ctl(Context ctx, unsigned op);
    // virtual ~Handle();

    void dup() { ref_count_++; };
    isize refcount() { return ref_count_; }
    void release();

private:
    virtual void close() = 0;
    isize ref_count_ = 0;
};
