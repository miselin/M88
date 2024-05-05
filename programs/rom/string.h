#ifndef _STRING_H
#define _STRING_H

#include <stdint.h>

void putchar(char c);
void puts(const char *s);
void puthex(uint16_t n);
void putnum(uint16_t n);

int strlen(const char *s);

void serial_putchar(char c);
void serial_puts(const char *s);
void serial_putnum(uint16_t n);
void serial_puthex(uint16_t n);

#endif
