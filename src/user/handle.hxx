#pragma once

#include "klib.hxx"

struct Handle {
    virtual result<isize> read(void* buffer, isize len);
    virtual result<isize> write(const void* data, isize len);
    // virtual ~Handle();

    void dup() { ref_count_++; };
    isize refcount() { return ref_count_; }
    void release();

private:
    virtual void close() = 0;
    isize ref_count_ = 0;
};
