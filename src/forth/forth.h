#pragma once

#include "kdef.h"

#ifdef __cplusplus
extern "C" {
#endif

struct Forth_header {
    const struct Forth_header* next;
    bool immediate;
    u8 name_length;
    char name[];
} __attribute__((packed));

struct Forth_body {
    void* code;
    isize parameters;
};

typedef struct Forth_body* Forth_xt;

struct Forth_context {
    void* here;
    const struct Forth_header* dictionary;
    const char* tib;
    isize ntib;
    isize to_in;
    isize state;
};

Forth_xt forth_header_to_xt(const struct Forth_header* h);
isize forth_exec(struct Forth_context*, const Forth_xt program[], isize stack[], isize stack_count);
int forth_interpret(const char* text, isize chars, isize stack[]);
void forth_init();

extern const struct Forth_header* forth_headers;

#ifdef __cplusplus
}
#endif
