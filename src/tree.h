#pragma once

#include "kdef.h"

enum tree_dir : bool { TREE_LEFT, TREE_RIGHT };

struct Tree_node {
    struct Tree_node* parent;
    union {
        struct {
            struct Tree_node* left;
            struct Tree_node* right;
        };
        struct Tree_node* child[2];
    };

    isize key;
    int priority;
    enum tree_dir dir;
};

struct Tree {
    struct Tree_node* root;
};

void tree_insert(struct Tree* tree, struct Tree_node* node, isize key);
void tree_insert_at(struct Tree* tree, struct Tree_node* parent, enum tree_dir dir, struct Tree_node* child, isize key);
void tree_remove(struct Tree* tree, struct Tree_node* node);
struct Tree_node* tree_begin(struct Tree* tree);
struct Tree_node* tree_next(struct Tree_node* node);
// struct Tree_node* tree_prev(struct Tree_node* node);
