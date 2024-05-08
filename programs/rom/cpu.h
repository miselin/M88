#ifndef _CPU_H
#define _CPU_H

#include <stdint.h>

#define F_CF (1 << 0)
#define F_PF (1 << 2)
#define F_AF (1 << 4)
#define F_ZF (1 << 6)
#define F_SF (1 << 7)
#define F_TF (1 << 8)
#define F_IF (1 << 9)
#define F_DF (1 << 10)
#define F_OF (1 << 11)
#define F_IOPL (3 << 12)
#define F_NT (1 << 14)
#define F_RF (1 << 15)

#pragma pack(1)

struct isrstack {
  uint16_t isrnum;
  union {
    struct {
      uint16_t bp;
      uint16_t si;
      uint16_t di;
      uint16_t es;
      uint16_t ds;
      uint16_t dx;
      uint16_t cx;
      uint16_t bx;
      uint16_t ax;
      uint16_t ip;
      uint16_t cs;
      uint16_t flags;
    } regs;
    uint16_t stack[12];
  };
};

#pragma pack()

#endif