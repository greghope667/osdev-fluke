#pragma once

#include "memory.h"

void pmm_add_pages(physical_t start, isize length);
physical_t pmm_alloc_page_phys();
void pmm_print_info();

void* alloc_page();
void free_page(void* page);
