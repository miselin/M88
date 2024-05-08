#include <dos.h>

#include "bda.h"

struct bda_t __far *bda = MK_FP(0x0000, 0x0000);

static int serialports[4] = {0x3F8, 0x2F8, 0x3E8, 0x2E8};

void configure_serial() {
  for (int i = 0; i < 4; ++i) {
    bda->comport[i] = 0;

    outportb(serialports[i] + 1, 0x00);  // Disable all interrupts
    outportb(serialports[i] + 3, 0x80);  // Enable DLAB (set baud rate divisor)
    outportb(serialports[i] + 0,
             0x03);  // Set divisor to 3 (lo byte) 38400 baud
    outportb(serialports[i] + 1, 0x00);  //                  (hi byte)
    outportb(serialports[i] + 3, 0x03);  // 8 bits, no parity, one stop bit
    outportb(serialports[i] + 2,
             0xC7);  // Enable FIFO, clear them, with 14-byte threshold
    outportb(serialports[i] + 4, 0x0B);  // IRQs enabled, RTS/DSR set

    outportb(serialports[i] + 4,
             0x1E);  // Set in loopback mode, test the serial chip

    outportb(serialports[i] + 0, 0xAE);  // Test serial chip (send byte 0xAE and
                                         // check if serial returns same byte)

    // Check if serial is faulty (i.e: not same byte as sent)
    if (inportb(serialports[i] + 0) != 0xAE) {
      continue;
    }

    // If serial is not faulty set it in normal operation mode
    // (not-loopback with IRQs enabled and OUT#1 and OUT#2 bits enabled)
    outportb(serialports[i] + 4, 0x0F);

    bda->comport[i] = serialports[i];
  }
}
