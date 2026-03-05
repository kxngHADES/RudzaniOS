#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>

void mem_init(void);
void *mem_alloc_page(void);
void mem_free_page(void *page);
uint32_t mem_get_total_pages(void);
uint32_t mem_get_free_pages(void);
uint32_t mem_get_used_pages(void);

#endif
