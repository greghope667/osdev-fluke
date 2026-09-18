#pragma once

#include "handle.hxx"
#include "containers/tree.hxx"

struct IPC {
    Tree::Node tree;
    static result<IPC*> create(i8 transfer_map[], usize ntransfer_map);
    error_code          listen(Registers* ctx);
    static result<void> respond(Registers* ctx);
    void                close(Tree&);
    Handle*             handle();
};
