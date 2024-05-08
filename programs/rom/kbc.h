#ifndef _KBC_H
#define _KBC_H

void kbc_write(unsigned char value, int offset);
unsigned short kbc_read(int offset);
void configure_kbc();

#endif