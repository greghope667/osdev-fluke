#pragma once

#define GDT_KERNEL_CODE     0x08
#define GDT_KERNEL_DATA     0x10
#define GDT_USER_DATA       0x23
#define GDT_USER64_CODE     0x2b

#define CPU_OFFSET_THIS         0x00
#define CPU_OFFSET_KERNEL_SP    0x08
#define CPU_OFFSET_USER_SP      0x10
