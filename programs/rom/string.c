#include "string.h"

#include <dos.h>
#include <stdarg.h>

extern int vsprintf(char __ss *buf, const char *fmt, va_list args);

int strlen(const char *s) {
  int len = 0;
  while (*s++) {
    ++len;
  }
  return len;
}

int printf(const char *fmt, ...) {
  char print_temp[256];
  va_list argptr;
  va_start(argptr, fmt);
  int ret = vsprintf(print_temp, fmt, argptr);
  write_bochs_ss(print_temp);
  va_end(argptr);
  return ret;
}

int snprintf(char __ss *s, int n, const char *fmt, ...) {
  va_list argptr;
  va_start(argptr, fmt);
  int ret = vsprintf(s, fmt, argptr);
  va_end(argptr);
  return ret;
}

int sprintf(char __ss *s, const char *fmt, ...) {
  va_list argptr;
  va_start(argptr, fmt);
  int ret = vsprintf(s, fmt, argptr);
  va_end(argptr);
  return ret;
}
