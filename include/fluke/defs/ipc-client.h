#pragma once

#include "ipc.h"

#define IPC_CLASS_SEEK                  ( 1 << IPC_CLASS_SHIFT)

#define IPC_SEEK_LSEEK                  ( 1 << IPC_METHOD_SHIFT)
#define IPC_SEEK_PREAD                  ( 2 << IPC_METHOD_SHIFT)
#define IPC_SEEK_PWRITE                 ( 3 << IPC_METHOD_SHIFT)


#define IPC_CLASS_DIR                   ( 2 << IPC_CLASS_SHIFT)

#define IPC_DIR_OPENAT                  ( 1 << IPC_METHOD_SHIFT)
#define IPC_DIR_GETDENTS                ( 2 << IPC_METHOD_SHIFT)


// ipc_lseek(fd, offset, whence, %)
#define IPC_lseek                       (IPC_CLASS_SEEK | IPC_SEEK_LSEEK)

// ipc_pread(fd, ptr, len, %, offset)
#define IPC_pread                       (IPC_CLASS_SEEK | IPC_SEEK_PREAD | IPC_CALL_RXSTR)

// ipc_pwrite(fd, ptr, len, %, offset)
#define IPC_pwrite                      (IPC_CLASS_SEEK | IPC_SEEK_PWRITE | IPC_CALL_TXSTR | IPC_CALL_FRAGMENT)

// ipc_openat(dirfd, path, pathlen, %, oflag, mode)
#define IPC_openat                      (IPC_CLASS_DIR | IPC_DIR_OPENAT | IPC_CALL_TXSTR | IPC_CALL_RXOPEN)
