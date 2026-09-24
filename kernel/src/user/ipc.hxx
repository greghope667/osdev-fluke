#pragma once

#include "handle.hxx"
#include "containers/tree.hxx"

struct IPC {
    static constexpr int TRANSFER_MAX = 5;
    Tree::Node tree;
    static result<IPC*> create(u8 transfer_map[TRANSFER_MAX]);
    error_code          listen(Registers* ctx);
    static result<void> respond(Registers* ctx);
    void                close(Tree&);
    Handle*             handle();
};
