#pragma once

#include "mem/vm.h"
#include "x86_64/cpu.h"
#include "queue.h"
#include "tree.h"

enum process_state {
    SPAWNING,
    FLOATING,
    RUNNING,
    READY,
    WAITING,
};

struct Process {
    struct VM vm;
    struct Context saved_context;
    struct Queue_node queue;
    enum process_state state;
    struct Queue_node timeout;
    struct Tree descriptors;
};

struct Process* process_create();
void process_load_flat_binary(struct Process* process, const void* binary, usize size);
