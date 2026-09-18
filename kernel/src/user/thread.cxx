#include "process.hxx"

void
Thread::push_into(Queue* q)
{
    assert(state == FLOATING);
    assert(!queue.root);
    assert(!timeout.prev);
    assert(!timeout.next);
    state = WAITING;
    queue_push(q, &queue);
}

Thread*
Thread::pop_from(Queue* q)
{
    auto node = queue_pop(q);
    if (not node)
        return nullptr;

    auto& thread = *thread_cast(node);
    assert(thread.state == WAITING);
    thread.state = FLOATING;
    thread.timeout.cancel();
    return &thread;
}


void
Thread::Timeout::cancel()
{
    if (prev) {
        if (next)
            next->timeout.prev = prev;
        *prev = next;
        *this = {};
    }
}
