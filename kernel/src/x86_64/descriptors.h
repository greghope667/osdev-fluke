#pragma once

#include "kdef.h"

void x86_64_load_early_descriptors();
void x86_64_load_descriptors(usize exception_stack);
