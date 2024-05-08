#include "io.h"

void configure_pic() {
  // ICW1: ICW4 needed, cascade mode, 4-byte interval, edge-triggered
  outportb(0x20, 0x15);
  outportb(0xA0, 0x15);
  // ICW2: remap IRQ0 to interrupt 8, IRQ8 to interrupt 70
  outportb(0x21, 0x08);
  outportb(0xA1, 0x70);
  // ICW3: IRQ2 is the cascade
  outportb(0x21, 0x04);
  outportb(0xA1, 0x02);
  // ICW4: 8086 mode, normal EOI
  outportb(0x21, 0x81);
  outportb(0xA1, 0x81);

  // mask all interrupts
  outportb(0x21, 0xFF);
  outportb(0xA1, 0xFF);
}
