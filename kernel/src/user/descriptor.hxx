#pragma once

#include "klib.hxx"
#include "mem/alloc.hxx"
#include "handle.hxx"

struct Descriptor {
    Handle* handle;
    bool open;
    // bool cloexec;
    // u32 flags;

    void assign(Handle*);
    void close();
    ~Descriptor() { if (open) handle->release(); }
};

// struct Descriptor_table_l2 {
    // struct Descriptor_table_l1* l1[32];
// };

struct Descriptor_table {
    static constexpr int L0 = 8;
    static constexpr int L1 = 4;
    static constexpr int L1L0 = 16;

    using table_l0 = std::array<Descriptor, L0>;

    using table_l1l0 = std::array<Descriptor, L1L0>;
    using table_l1 = std::array<owned<table_l1l0>, L1>;

    table_l0 l0 = {};
    table_l1 l1 = {};
    // struct Descriptor_table_l2* l2[4];

    result<Descriptor*> alloc(int& fd_out);
    result<Descriptor*> get(int fd);
    result<void> close(int fd);
    result<void> clone(Descriptor_table& to);
};

// struct Descriptor* descriptor_reserve(struct Descriptor_table*, int* fd);
