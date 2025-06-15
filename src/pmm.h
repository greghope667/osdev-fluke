#pragma once

#include "memory.h"

void pmm_add_pages(physical_t start, isize length);
void* alloc_page();
physical_t pmm_alloc_page_phys();
void free_page(void* page);
void pmm_print_info();
