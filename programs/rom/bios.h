#ifndef _BIOS_H
#define _BIOS_H

#include <stdint.h>

#include "cpu.h"

void install_bios_isrs(uint16_t cs);
int handle_bios_interrupt(struct isrstack __ss *stack, unsigned short isrnum);

#endif