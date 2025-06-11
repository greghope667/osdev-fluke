#pragma once

#include "kdef.h"

struct Cpuid_result {
    u32 eax, ebx, ecx, edx;
};

static inline struct Cpuid_result
cpuid(u32 function, u32 parameter)
{
    u32 eax = function, ecx = parameter;
    u32 ebx, edx;
    asm volatile ("cpuid" : "+a"(eax), "=b"(ebx), "+c"(ecx), "=d"(edx));
    return (struct Cpuid_result){ eax, ebx, ecx, edx };
}
