#include "pit.h"

#include <dos.h>

#include "bda.h"
#include "fdc.h"
#include "io.h"

struct bda_t __far *bda = MK_FP(0x0000, 0x0000);

void configure_pit() {
  // Channel 0 - lo/hi byte, rate generator, binary mode
  outportb(0x43, 0x36);

  // 18.2 Hz
  outportb(0x40, 0xFF);
  outportb(0x40, 0xFF);

  // unmask timer interrupt
  outportb(0x21, 0xFE);

  bda->daycounter = 0;
  bda->daycounter2 = 0;
  bda->timer = 0;
  bda->clockrollover = 0;
}

void pit_irq() {
  bda->timer++;

  if (bda->motorshutoff) {
    bda->motorshutoff--;

    if (!bda->motorshutoff) {
      fdc_motor_off();
    }
  }
}

uint16_t read_pit_counter() {
  asm { cli }

  outportb(0x43, 0x00);  // latch value
  uint8_t lo = inportb(0x40);
  uint8_t hi = inportb(0x40);

  asm { sti }

  return lo | (hi << 8);
}

void delay_ticks(uint16_t ticks) {
  uint16_t start = bda->timer;

  // counter ticks downwards
  while ((bda->timer - start) < ticks) {
    // wait
    asm { hlt }
  }
}
