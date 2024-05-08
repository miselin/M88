#ifndef _INTS_H
#define _INTS_H

#include <stdint.h>

#include "cpu.h"
#include "io.h"
#include "string.h"

void irqhandler(unsigned short irqnum);
void isrhandler(struct isrstack __ss *stack, unsigned short isrnum);

#endif
