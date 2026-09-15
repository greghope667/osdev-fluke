#include "tree.hxx"

#include "klib.h"

// typedef struct Tree_node Node;
// typedef struct Tree Tree;
using Node = Tree::Node;
#define DIR_LEFT false
#define DIR_RIGHT true

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
Tree::insert_at(Node* parent, direction dir, Node* child)
{
    if (not parent)
        return insert_root(child);

    assert(parent->child[dir] == nullptr);
    assert(child->parent == nullptr);

    parent->child[dir] = child;

    *child = (Node) {
        .parent = parent,
        .child = {},
        .priority = rng(),
        .dir = dir,
    };

    if (child->priority > parent->priority)
        rotate_up(this, parent, child);
}

/*
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
*/
void
Tree::insert_root(Node* node)
{
    assert(root == nullptr);
    assert(node->parent == nullptr);

    root = node;
    *node = (Node) {
        .parent = nullptr,
        .child = {},
        .priority = rng(),
        .dir = {},
    };
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
Tree::remove(Node* node)
{
    assert(root);
    Node** parent_ptr = node->parent ? &node->parent->child[node->dir] : &root;
    tree_remove_at(parent_ptr, node);
    memset(node, 0, sizeof(Node));
}

static inline Node*
leftmost(Node* node)
{
    while (node->left)
        node = node->left;
    return node;
}

Node*
Tree::begin()
{
    if (not root)
        return nullptr;
    return leftmost(root);
}

Node*
Tree::Node::next()
{
    if (right)
        return leftmost(right);
    auto n = this;
    while (n->dir == DIR_RIGHT)
        n = n->parent;
    return n->parent;
}
