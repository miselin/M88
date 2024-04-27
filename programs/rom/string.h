#ifndef _STRING_H
#define _STRING_H

int strlen(const char *s);
int printf(const char *fmt, ...);
int snprintf(char __ss *s, int n, const char *fmt, ...);
int sprintf(char __ss *s, const char *fmt, ...);

void write_bochs_char(char c);
void write_bochs(const char *s);
void write_bochs_ss(const char __ss *s);

#endif
