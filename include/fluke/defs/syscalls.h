#pragma once

#define SYSCALL_nop                 0x1200
#define SYSCALL_forth_interpret     0x1201
#define SYSCALL_nsleep              0x1202
#define SYSCALL_open_module         0x1203
#define SYSCALL_user_share          0x1204
#define SYSCALL_claim_irq           0x1205
#define SYSCALL_klog                0x1206
#define SYSCALL_panic               0x1207
#define SYSCALL_fork                0x1208
#define SYSCALL_read                0x1210
#define SYSCALL_seek                0x1212
#define SYSCALL_objctl              0x121f
#define SYSCALL_virtual_map         0x1220
#define SYSCALL_virtual_unmap       0x1221
#define SYSCALL_virtual_protect     0x1222
#define SYSCALL_process_exit        0x1230
#define SYSCALL_thread_spawn        0x1231
#define SYSCALL_ipc_create          0x1240
#define SYSCALL_ipc_call            0x1241
#define SYSCALL_ipc_listen          0x1242
#define SYSCALL_ipc_respond         0x1243
