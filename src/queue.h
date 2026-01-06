#pragma once

#include "klib.h"

struct Queue;

struct Queue_node {
    struct Queue_node* next;
    struct Queue_node* prev;
    struct Queue* root;
};

struct Queue {
    struct Queue_node* head;
    struct Queue_node* tail;
};

struct Queue_node* queue_pop(struct Queue* queue);
void queue_push(struct Queue* queue, struct Queue_node* node);
void queue_node_remove(struct Queue_node* node);
