#pragma once

struct Descriptor {
    struct Handle* handle;
    bool open;
    // bool cloexec;
    // u32 flags;
};

struct Descriptor_table_l1 {
    struct Descriptor l0[16];
};

// struct Descriptor_table_l2 {
    // struct Descriptor_table_l1* l1[32];
// };

struct Descriptor_table {
    struct Descriptor l0[8];
    struct Descriptor_table_l1* l1[4];
    // struct Descriptor_table_l2* l2[4];
};

struct Descriptor* descriptor_new(struct Descriptor_table*, int* fd);
struct Descriptor* descriptor_get(struct Descriptor_table*, int fd);
bool descriptor_close(struct Descriptor_table*, int fd);
// struct Descriptor* descriptor_reserve(struct Descriptor_table*, int* fd);
void descriptor_assign(struct Descriptor*, struct Handle*);
