#include <stdint.h>

#include "cpu.h"
#include "io.h"
#include "string.h"

void irqhandler(unsigned short irqnum) {
  if (irqnum == 0) {
    // timer
    pit_irq();
  } else if (irqnum == 6) {
    // floppy
    fdc_irq();
  }

  if (irqnum >= 8) {
    outportb(0xA0, 0x20);
  }
  outportb(0x20, 0x20);
}

void isrhandler(struct isrstack __ss *stack, unsigned short isrnum) {
  if (isrnum >= 8 && isrnum <= 15) {
    irqhandler(isrnum - 8);
    return;
  }

  if (!handle_bios_interrupt(stack, isrnum)) {
    return;
  }

  puts("ISR ");
  puthex(isrnum);
  puts("\r\n");

  // dump regs
  for (int i = 0; i < 12; ++i) {
    puthex(stack->stack[i]);
    puts(" ");
  }

  puts("\r\n");
  puts("CS:IP ");
  puthex(stack->regs.cs);
  puts(":");
  puthex(stack->regs.ip);
  puts("\r\n");

  panic("Unhandled ISR\n");
}
