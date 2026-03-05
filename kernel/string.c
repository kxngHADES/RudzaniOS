#include "string.h"

size_t strlen(const char *str) {
	size_t len = 0;
	while (str[len]) {
		len++;
	}
	return len;
}

int strcmp(const char *s1, const char *s2) {
	while (*s1 && (*s1 == *s2)) {
		s1++;
		s2++;
	}
	return *(unsigned char *)s1 - *(unsigned char *)s2;
}

int strncmp(const char *s1, const char *s2, size_t n) {
	while (n && *s1 && (*s1 == *s2)) {
		s1++;
		s2++;
		n--;
	}
	if (n == 0) {
		return 0;
	}
	return *(unsigned char *)s1 - *(unsigned char *)s2;
}

char *strcpy(char *dest, const char *src) {
	char *d = dest;
	while ((*d++ = *src++));
	return dest;
}

char *strncpy(char *dest, const char *src, size_t n) {
	size_t i;
	for (i = 0; i < n && src[i] != '\0'; i++) {
		dest[i] = src[i];
	}
	for (; i < n; i++) {
		dest[i] = '\0';
	}
	return dest;
}

char *strcat(char *dest, const char *src) {
	char *d = dest;
	while (*d) d++;
	while ((*d++ = *src++));
	return dest;
}

char *strncat(char *dest, const char *src, size_t n) {
	char *d = dest;
	while (*d) d++;
	while (n-- && *src) {
		*d++ = *src++;
	}
	*d = '\0';
	return dest;
}


void *memset(void *dest, int val, size_t count) {
	unsigned char *d = (unsigned char *)dest;
	while (count--) {
		*d++ = (unsigned char)val;
	}
	return dest;
}

void *memcpy(void *dest, const void *src, size_t count) {
	unsigned char *d = (unsigned char *)dest;
	const unsigned char *s = (const unsigned char *)src;
	while (count--) {
		*d++ = *s++;
	}
	return dest;
}

void itoa(int value, char *buf, int base) {
	char *p = buf;
	char *p1, *p2;
	unsigned int uvalue;
	int negative = 0;

	if (base < 2 || base > 16) {
		*buf = '\0';
		return;
	}

	if (value < 0 && base == 10) {
		negative = 1;
		uvalue = (unsigned int)(-(value + 1)) + 1;
	} else {
		uvalue = (unsigned int)value;
	}

	/* Generate digits in reverse order */
	do {
		unsigned int remainder = uvalue % base;
		*p++ = (remainder < 10) ? '0' + remainder : 'A' + remainder - 10;
		uvalue /= base;
	} while (uvalue);

	if (negative) {
		*p++ = '-';
	}

	*p = '\0';

	/* Reverse the string */
	p1 = buf;
	p2 = p - 1;
	while (p1 < p2) {
		char tmp = *p1;
		*p1 = *p2;
		*p2 = tmp;
		p1++;
		p2--;
	}
}

void ftoa(double value, char *buf, int num_decimals) {
	if (value < 0.0) {
		*buf++ = '-';
		value = -value;
	}

	int integer_part = (int)value;
	double fraction_part = value - (double)integer_part;

	itoa(integer_part, buf, 10);
	while (*buf) buf++;

	if (num_decimals > 0) {
		*buf++ = '.';
		while (num_decimals-- > 0) {
			fraction_part *= 10.0;
			int digit = (int)fraction_part;
			*buf++ = '0' + digit;
			fraction_part -= digit;
		}
	}
	*buf = '\0';
}
