
#include "io.h"

void outportb(uint16_t port, uint8_t value) {
  asm {
    mov dx, port
    mov al, value
    out dx, al
  }
}

uint8_t inportb(uint16_t port) {
  asm {
    xor ax, ax
    mov dx, port
    in al, dx
  }
}
