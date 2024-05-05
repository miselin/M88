#ifndef _FDC_H
#define _FDC_H

#include <dos.h>
#include <stdint.h>

int configure_fdc();

void fdc_irq();

int fdc_motor_off();
int fdc_motor_on();

int fdc_read_drive0(uint8_t head, uint8_t cyl, uint8_t sector, uint16_t seg,
                    uint16_t addr, uint16_t length);

#endif