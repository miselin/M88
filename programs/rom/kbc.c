#include "kbc.h"

#include "io.h"
#include "string.h"

static void kbc_wait_write() {
  while (inportb(0x64) & 0x02) {
    // wait
  }
}

static void kbc_wait_read() {
  while (!(inportb(0x64) & 0x01)) {
    // wait
  }
}

void kbc_write(unsigned char value, int offset) {
  kbc_wait_write();
  outportb(0x60 + offset, value);
}

unsigned short kbc_read(int offset) {
  kbc_wait_read();
  return inportb(0x60 + offset);
}

void configure_kbc() {
  // disable all PS/2 devices
  kbc_write(0xAD, 4);
  kbc_write(0xA7, 4);

  // disable IRQs
  kbc_write(0x20, 4);
  unsigned char config = kbc_read(0);
  config = (config & ~3) | 0x40;
  kbc_write(0x60, 4);
  kbc_write(config, 0);

  // self-test
  kbc_write(0xAA, 4);
  if (kbc_read(0) != 0x55) {
    puts("KBC self-test failed, no KBC\n");
    return;
  }

  // enable ports
  kbc_write(0xAE, 4);
  kbc_write(0xA8, 4);

  // reset devices
  while (1) {
    kbc_write(0xFF, 0);
    unsigned char result = kbc_read(0);
    if (result == 0xFE) {
      continue;
    }

    if (result != 0xFA) {
      panic("KBC reset failed 1\n");
    }

    result = kbc_read(0);
    if (result != 0xAA) {
      panic("KBC reset failed 2\n");
    }

    break;
  }

  // prove it works by turning on num lock
  kbc_write(0xED, 0);
  kbc_write(0x02, 0);
  kbc_read(0);  // consume ack
}
