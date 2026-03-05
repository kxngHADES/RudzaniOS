#ifndef PAGING_H
#define PAGING_H

#include <stdint.h>

/* Page directory entry flags */
#define PAGE_PRESENT  0x01
#define PAGE_WRITABLE 0x02
#define PAGE_USER     0x04

void paging_init(void);
void paging_switch_directory(uint32_t *directory);
void paging_enable(void);

#endif
