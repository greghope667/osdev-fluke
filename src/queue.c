#include "queue.h"

#include "klib.h"

struct Queue_node*
queue_pop(struct Queue* queue)
{
    struct Queue_node* node = queue->head;
    if (!node)
        return nullptr;

    assert(node->prev == nullptr);
    assert(node->root == queue);

    queue->head = node->next;
    if (queue->tail == node)
        queue->tail = nullptr;

    return memset(node, 0, sizeof(*node));
}

void
queue_push(struct Queue* queue, struct Queue_node* node)
{
    assert(node->root == nullptr);

    *node = (struct Queue_node) {
        .root = queue,
        .prev = queue->tail,
        .next = nullptr,
    };

    queue->tail = node;
    if (!queue->head)
        queue->head = node;
}

void
queue_node_remove(struct Queue_node* node)
{
    struct Queue* queue = node->root;
    assert(queue);

    if (node->prev)
        node->prev->next = node->next;
    else
        queue->head = node->next;

    if (node->next)
        node->next->prev = node->prev;
    else
        queue->tail = node->prev;

    memset(node, 0, sizeof(*node));
}
