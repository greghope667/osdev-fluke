#pragma once

#define SYSCALL_nop                 0x1200
#define SYSCALL_forth_interpret     0x1201
#define SYSCALL_nsleep              0x1202
#define SYSCALL_open_module         0x1203
#define SYSCALL_read                0x1204
#define SYSCALL_virtual_map         0x1205
#define SYSCALL_virtual_unmap       0x1206
#define SYSCALL_virtual_protect     0x1207


#define EINVAL                      1
#define ENOSYS                      2
#define EIO                         3
#define ENOMEM                      4
#define ENOENT                      5
#define ENAMETOOLONG                6
#define EMFILE                      7
#define EBADF                       8
#define EFAULT                      9


#define PROT_NONE                   0
#define PROT_READ                   (1 << 0)
#define PROT_WRITE                  (1 << 1)
#define PROT_EXEC                   (1 << 2)


#define MAP_FIXED                   (1 << 0)
