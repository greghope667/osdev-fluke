#pragma once

#include "vm.hxx"
#include "x86_64/cpu.h"
#include "containers/queue.h"
#include "containers/tree.hxx"
#include "descriptor.hxx"

struct Thread : Thread_context {
    struct Timeout {
        Thread** prev;
        Thread* next;
        u64 ns;
    } timeout;

    Queue_node queue;

    Tree::Node tree;
    struct Process* process;

    struct Process& get_process();
};

static inline __attribute__((always_inline))
Thread* thread_cast(Thread_context* ctx)
{
    return static_cast<Thread*>(ctx);
}

static inline __attribute__((always_inline))
Thread* thread_cast(Queue_node* node)
{
    return container_of(node, Thread, queue);
}

static inline __attribute__((always_inline))
Thread* thread_cast(Tree::Node* node)
{
    return container_of(node, Thread, tree);
}

struct Process {
    enum State {
        SPAWNING,
        ACTIVE,
        DEAD,
    };

    VM vm;
    Descriptor_table descriptors;
    Tree threads;
    State state;

    static result<Process*> create();
    result<Thread*> spawn_thread(usize code, usize stack, usize arg);

    VM& get_vm();
};
