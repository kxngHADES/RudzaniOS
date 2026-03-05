#ifndef STRING_H
#define STRING_H

#include <stdint.h>
#include <stddef.h>

size_t strlen(const char *str);
int strcmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, size_t n);
char *strcpy(char *dest, const char *src);
char *strncpy(char *dest, const char *src, size_t n);
char *strcat(char *dest, const char *src);
char *strncat(char *dest, const char *src, size_t n);
void *memset(void *dest, int val, size_t count);
void *memcpy(void *dest, const void *src, size_t count);
void itoa(int value, char *buf, int base);
void ftoa(double value, char *buf, int num_decimals);

#endif
