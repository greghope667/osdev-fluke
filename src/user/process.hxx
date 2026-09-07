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

    Queue_node queue;

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

    VM& get_vm();
};
