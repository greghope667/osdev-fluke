#include "tree.h"

#include "klib.h"
#include "panic.h"

typedef struct Tree_node Node;
typedef struct Tree Tree;

static int
rng()
{
    static u64 state;
    state = state * 6364136223846793005ull + 1442695040888963407ull;
    return state >> 32;
}

static void
rotate_up(Tree* tree, Node* parent, Node* child)
{
    /*        G                 G
     *      /  \              /  \
     *     P                 C
     *   /  \       -->    /  \
     *  x    C            P    z
     *      / \          / \
     *     y   z        x   y
     *
     * Chart for case dir == RIGHT
     */
    bool dir = child->dir;

    Node* G = parent->parent;
    child->parent = G;
    child->dir = parent->dir;

    Node* y = child->child[!dir];
    parent->child[dir] = y;
    if (y) {
        y->parent = parent;
        y->dir = dir;
    }

    child->child[!dir] = parent;
    parent->parent = child;
    parent->dir = !dir;

    if (G) {
        G->child[child->dir] = child;
        if (child->priority > G->priority)
            rotate_up(tree, G, child);
    } else {
        tree->root = child;
    }
}

void
tree_insert_at(
    Tree* tree,
    Node* parent,
    enum tree_dir dir,
    Node* child,
    isize key
) {
    assert(parent->child[dir] == nullptr);

    parent->child[dir] = child;

    *child = (Node) {
        .dir = dir,
        .key = key,
        .parent = parent,
        .priority = rng(),
    };

    if (child->priority > parent->priority)
        rotate_up(tree, parent, child);
}

void
tree_insert(
    Tree* tree,
    Node* node,
    isize key
) {
    Node* parent = tree->root;
    if (!parent) {
        tree->root = node;
        *node = (Node) {
            .key = key,
            .priority = rng(),
        };
    } else {
        for (;;) {
            bool dir = key > parent->key;
            Node* next = parent->child[dir];
            if (!next) return tree_insert_at(tree, parent, dir, node, key);
            parent = next;
        }
    }
}

static void
tree_remove_at(
    Node** parent_ptr,
    Node* node
) {
    assert(*parent_ptr == node);

    if (!node->left && !node->right) {
        *parent_ptr = nullptr;
        return;
    }

    if (node->left && !node->right) {
        *parent_ptr = node->left;
        node->left->parent = node->parent;
        node->left->dir = node->dir;
        return;
    }

    if (!node->left && node->right) {
        *parent_ptr = node->right;
        node->right->parent = node->parent;
        node->right->dir = node->dir;
        return;
    }

    /*          |             |
     *          N             C
     *        /  \           / \
     *       C    z         x   N
     *      / \       -->      / \
     *     x  y               y   z
     *
     * dir == RIGHT
     */

    bool dir = node->left->priority > node->right->priority;

    Node* C = node->child[!dir];
    Node* y = C->child[dir];

    *parent_ptr = C;
    C->parent = node->parent;
    C->dir = node->dir;

    C->child[dir] = node;
    node->dir = dir;
    node->parent = C;

    node->child[!dir] = y;
    if (y) {
        y->dir = !dir;
        y->parent = node;
    }

    tree_remove_at(&C->child[dir], node);
}

void
tree_remove(
    Tree* tree,
    Node* node
) {
    assert(tree->root);
    Node** parent_ptr = node->parent ? &node->parent->child[node->dir] : &tree->root;
    tree_remove_at(parent_ptr, node);
    memset(node, 0, sizeof(Node));
}
