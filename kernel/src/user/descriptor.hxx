#pragma once

#include "klib.hxx"
#include "mem/alloc.hxx"
#include "handle.hxx"

struct Descriptor {
    Handle* _handle;

    void assign(Handle*, bool cloexec);
    void close();

    void set_cloexec()      { _handle = (Handle*)((usize)_handle | 1zu); }
    void clear_cloexec()    { _handle = (Handle*)((usize)_handle & ~1zu); }
    bool is_cloexec()       { return (usize)_handle & 1; }
    Handle* handle()        { return (Handle*)((usize)_handle & ~1zu); }
    bool is_open()          { return _handle; }
    ~Descriptor()           { if (_handle) _handle->release(); }
};

struct Descriptor_table {
    static constexpr int L0 = 8;
    static constexpr int L1 = 4;
    static constexpr int L1L0 = 16;

    using table_l0 = std::array<Descriptor, L0>;

    using table_l1l0 = std::array<Descriptor, L1L0>;
    using table_l1 = std::array<owned<table_l1l0>, L1>;

    table_l0 l0 = {};
    table_l1 l1 = {};

    result<Descriptor*> alloc(int& fd_out);
    result<Descriptor*> alloc_overwrite(int fd);
    result<Descriptor*> get(int fd);
    result<void> close(int fd);
    result<void> clone(Descriptor_table& to);
    void cloexec();
};

// struct Descriptor* descriptor_reserve(struct Descriptor_table*, int* fd);
