#include <dos.h>
#include <stdint.h>

#include "bda.h"
#include "bios.h"
#include "cpu.h"
#include "fdc.h"
#include "io.h"
#include "kbc.h"
#include "pit.h"
#include "serial.h"
#include "string.h"

struct bda_t __far *bda = MK_FP(0x0000, 0x0000);

typedef void(__far *optionrom_entry_t)();

extern unsigned short isr_array[256];

void jump_bootloader();

void update_7seg(unsigned char value) { outportb(0xE0, value); }

void panic(const char *s) {
  puts("PANIC: ");
  puts(s);
  puts("\n");
  asm { cli; hlt }
}

extern void call_option_roms();

extern uint16_t isr_array[256];

void relocate_stack();

void initialize_bda();

static void count_memory() {
  // start at 64K, count in 1K blocks
  uint8_t __far *mem = MK_FP(0x1000, 0x0000);

  uint16_t count = 64;

  puts("Memory check...\r\n");
  while (1) {
    *mem = 0x55;
    if (*mem != 0x55) {
      break;
    }

    puts("\r");
    putnum(count++);
    puts("K...");

    mem = MK_FP(count * 0x40, 0x0000);
  }

  bda->memsize = count;

  puts(" OK\r\n");
}

int rommain(uint16_t cs) {
  // Operating as an Option ROM - useful for testing
  // when the real system will have its own ROM for
  // INT 13 (disk operations)
  if (cs != 0xF000) {
    install_bios_isrs(cs);
    return 0;
  }

  initialize_bda();

  update_7seg(0x3);

  configure_pic();

  update_7seg(0x4);

  configure_pit();

  update_7seg(0x5);

  configure_serial();

  serial_puts("hello, world!\r\n");

  asm { sti }

  update_7seg(0x6);

  call_option_roms();

  update_7seg(0x7);

  // configure_kbc();

  update_7seg(0x8);

  count_memory();

  update_7seg(0x9);

  // we need to relocate the stack to the end of memory
  relocate_stack();

  update_7seg(0x10);

  for (int i = 0; i < 256; ++i) {
    if (bda->ivt[i].segment != 0xF000) {
      puts("IVT ");
      puthex(i);
      puts(": ");
      puthex(bda->ivt[i].segment);
      putchar(':');
      puthex(bda->ivt[i].offset);
      puts("\r\n");
    }
  }

  update_7seg(0x11);

  // configure FDC
  if (configure_fdc()) {
    panic("FDC configuration failed\n");
  }

  update_7seg(0x12);

  // set equipment flags
  bda->equipflags.fpu = 1;  // not on 8088 test machine
  bda->equipflags.mouse = 1;

  uint8_t __far *bootsector = MK_FP(0x0000, 0x7C00);
  fdc_read_drive0(0, 0, 1, 0, 0x7C00, 512);

  if (bootsector[510] != 0x55 || bootsector[511] != 0xAA) {
    puts("SIG: ");
    puthex(bootsector[510]);
    puthex(bootsector[511]);
    puts("\r\n");

    panic("Bootsector signature not found\n");
  }

  // jump to the bootloader
  jump_bootloader();

  return 0;
}

void assert(int cond) {
  if (!cond) {
    panic("assertion failed");
  }
}

void relocate_stack() {
  // initial stack is 0x0000:0xFFFF
  // we'll move it to the 4K at the top of real memory

  uint16_t new_stack_seg = ((bda->memsize - 4) * 0x40) & 0xF000;

  uint8_t __far *curr_stack = MK_FP(0x0000, 0xF000);
  uint8_t __far *new_stack = MK_FP(new_stack_seg, 0xF000);

  for (int i = 0; i < 0x1000; ++i) {
    new_stack[i] = curr_stack[i];
  }

  // remove last 4K from available memory, it's now the stack
  bda->memsize -= 4;

  // swap stack segment
  asm {
    mov ax, new_stack_seg;
    mov ss, ax
  }
}

void initialize_bda() {
  // load startup interrupt vectors
  for (int i = 0; i < 256; ++i) {
    bda->ivt[i].segment = 0xF000;
    bda->ivt[i].offset = isr_array[i];
  }

  bda->ivt[0x44].offset = 0xFA6E;

  bda->equipflags_raw = 0x0000;
  bda->equipflags.initialvid = 3;  // 80x25 16 color

  // assume 64K to start with, we'll count it later
  bda->memsize = 64;
}
