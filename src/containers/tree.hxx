#pragma once

struct Tree {
    // enum direction : bool { TREE_LEFT, TREE_RIGHT };
    using direction = bool;

    struct Node {
        Node* parent;

        union {
            struct {
                Node* left;
                Node* right;
            };
            Node* child[2];
        };

        int priority;
        direction dir;

        Node* next();
        // Node* prev();
    };

    Node* root;

    void insert_root(Node* child);
    void insert_at(Node* parent, direction dir, Node* child);
    void remove(Node* node);

    Node* begin();
};
