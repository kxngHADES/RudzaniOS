#ifndef PROCESS_H
#define PROCESS_H

#include <stdint.h>

#define MAX_PROCESSES 64

typedef struct {
	uint32_t pid;
	char name[32];
	int active;
} process_t;

void process_init(void);
uint32_t process_register(const char *name);
void process_unregister(uint32_t pid);
void process_list(void);

#endif
