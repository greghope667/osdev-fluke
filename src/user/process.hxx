#pragma once

#include "vm.hxx"
#include "x86_64/cpu.h"
#include "containers/queue.h"
#include "descriptor.hxx"

struct Thread : Thread_context {
    struct Timeout {
        Thread** prev;
        Thread* next;
        u64 ns;
    } timeout;

    struct Queue_node queue;

    struct Process& get_process();
};

struct Process {
    enum State {
        SPAWNING,
        ACTIVE,
        DEAD,
    };

    VM vm;
    Descriptor_table descriptors;
    Thread thread;
    State state;

    static result<Process*> create();

    void load_flat_binary(const char* binary, usize size);
    void load_init_elf(const char* elf);

    VM& get_vm();
};

// result<Process*> process_create();
// void process_load_flat_binary(struct Thread_context* process, const void* binary, usize size);
// void process_load_init_elf(struct Thread_context* process, const void* elf);
