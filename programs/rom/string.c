#include "string.h"

#include <dos.h>
#include <stdarg.h>
#include <stdint.h>

#include "io.h"

static char hexchars[] = "0123456789ABCDEF";

void putchar(char c) {
  asm {
    mov ah, 0x0E
    mov al, c
    mov bh, 0x00
    mov bl, 0x07
    int 0x10
  }
}

void putnum(uint16_t n) {
  char buf[5];
  int i = 0;
  do {
    buf[i++] = '0' + (n % 10);
    n /= 10;
  } while (n);
  while (i--) {
    putchar(buf[i]);
  }
}

void puthex(uint16_t n) {
  char buf[4];
  int i = 0;
  do {
    buf[i++] = hexchars[n % 16];
    n /= 16;
  } while (n);
  while (i--) {
    putchar(buf[i]);
  }
}

void puts(const char *s) {
  while (*s) {
    putchar(*s);
    ++s;
  }
}

int strlen(const char *s) {
  int len = 0;
  while (*s++) {
    ++len;
  }
  return len;
}

void serial_putchar(char c) {
  while ((inportb(0x3F8 + 5) & 0x20) == 0)
    ;

  outportb(0x3F8, c);
}

void serial_puts(const char *s) {
  while (*s) {
    serial_putchar(*s);
    ++s;
  }
}

void serial_putnum(uint16_t n) {
  char buf[5];
  int i = 0;
  do {
    buf[i++] = '0' + (n % 10);
    n /= 10;
  } while (n);
  while (i--) {
    serial_putchar(buf[i]);
  }
}

void serial_puthex(uint16_t n) {
  char buf[4];
  int i = 0;
  do {
    buf[i++] = hexchars[n % 16];
    n /= 16;
  } while (n);
  while (i--) {
    serial_putchar(buf[i]);
  }
}
