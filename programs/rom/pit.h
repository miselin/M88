#ifndef _PIT_H
#define _PIT_H

#include <stdint.h>

void configure_pit();

void pit_irq();

uint16_t read_pit_counter();
void delay_ticks(uint16_t ticks);

#endif
