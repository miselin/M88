#include <dos.h>
#include <stdint.h>

#include "bda.h"
#include "fdc.h"
#include "io.h"
#include "pit.h"
#include "string.h"

#pragma pack(1)

struct isrstack {
  uint16_t isrnum;
  union {
    struct {
      uint16_t bp;
      uint16_t si;
      uint16_t di;
      uint16_t es;
      uint16_t ds;
      uint16_t dx;
      uint16_t cx;
      uint16_t bx;
      uint16_t ax;
      uint16_t ip;
      uint16_t cs;
      uint16_t flags;
    } regs;
    uint16_t stack[12];
  };
};

#pragma pack()

struct bda_t __far *bda = MK_FP(0x0000, 0x0000);

typedef void(__far *optionrom_entry_t)();

extern unsigned short isr_array[256];

void jump_bootloader();

void blastall();

void update_7seg(unsigned char value) { outportb(0xE0, value); }

void panic(const char *s) {
  puts("PANIC: ");
  puts(s);
  puts("\n");
  asm { cli; hlt }
}

void configure_pic();
void configure_kbc();
extern void call_option_roms();

void disk_bios(struct isrstack __ss *stack);
void system_bios(struct isrstack __ss *stack);

extern uint16_t isr_array[256];

void relocate_stack();

void initialize_bda();

void configure_serial();

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

int rommain() {
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

  puts("hello, world\n");
  while (1) asm { sti; hlt }
    ;

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

  switch (isrnum) {
    case 0x10:
      //  INT 10 - Video BIOS Services
      //  no-op, should be installed by option ROMs
      return;
    case 0x11:
      // INT 11 - BIOS Equipment Determination
      stack->regs.ax = bda->equipflags_raw;
      return;
    case 0x12:
      // INT 12 - Memory Size Determination
      stack->regs.ax = 64;  // bda->memsize;
      return;
    case 0x13:
      disk_bios(stack);
      return;
    case 0x15:
      system_bios(stack);
      return;
  }

  /*
  serial_puts("ISR ");
  serial_puthex(isrnum);
  serial_puts(" is unhandled!\r\n");

  serial_puthex(stack->regs.cs);
  serial_puts(":");
  serial_puthex(stack->regs.ip);
  serial_puts("\r\n");
  */

  /*
  for (int i = 0; i < 256; ++i) {
    if (bda->ivt[i].segment != 0xF000) {
      serial_puts("IVT ");
      serial_puthex(i);
      serial_puts(": ");
      serial_puthex(bda->ivt[i].segment);
      serial_putchar(':');
      serial_puthex(bda->ivt[i].offset);
      serial_puts("\r\n");
    }
  }
  */

  update_7seg(isrnum | 0x40);

  asm { cli }
  while (1) {
    blastall();
  }

  asm {
    cli
    hlt
  }

  puts("ISR ");
  puthex(isrnum);
  puts("\r\n");

  // dump regs
  for (int i = 0; i < 12; ++i) {
    puthex(stack->stack[i]);
    puts(" ");
  }

  panic("Unhandled ISR\n");
}

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

void kbc_wait_write() {
  while (inportb(0x64) & 0x02) {
    // wait
  }
}

void kbc_wait_read() {
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

void assert(int cond) {
  if (!cond) {
    panic("assertion failed");
  }
}

void disk_bios(struct isrstack __ss *stack) {
  switch (stack->regs.ax >> 8) {
    case 0x00: {
      puts("DISK BIOS: reset disk system\r\n");
      /*
      // reset disk system
      uint8_t dl = stack->regs.dx & 0xFF;
      if (dl & 0x80) {
        panic("DISK BIOS: hard disk reset unimplemented\r\n");
      }

      if (dl != 0) {
        panic("DISK BIOS: reset disk not 0\r\n");
      }

      configure_fdc();
      stack->regs.ax = bda->diskstatus_raw;
      */

      stack->regs.ax = 0;
    } break;

    case 0x02: {
      // puts("DISK BIOS: read disk sectors\r\n");

      uint8_t cl = stack->regs.cx & 0xFF;
      uint8_t ch = stack->regs.cx >> 8;

      uint8_t sectors = stack->regs.ax & 0xFF;
      uint16_t cyl = ch | (((uint16_t)cl << 2) & 0x300);
      uint8_t head = stack->regs.dx >> 8;
      uint8_t sector = cl & 0x3F;
      uint8_t drive = stack->regs.dx & 0xFF;

      if (cyl > 80 || head > 1 || sector > 18) {
        puts("DISK BIOS: invalid CHS\r\n");
        puthex(ch);
        puts("  ");
        puthex(cl);
        puts("\r\n");
        // AH = status = 1 (bad command)
        stack->regs.ax = 0x100;
        // CF = 1 = failed
        stack->regs.flags |= 1;
        // invalid parameter
        bda->diskstatus.invalid = 1;
        return;
      }

      for (uint8_t count = 0; count < sectors; ++count) {
        if (fdc_read_drive0(head, cyl, sector + count, stack->regs.es,
                            stack->regs.bx + (count * 512), 512)) {
          puts("DISK BIOS: read error\r\n");
          stack->regs.ax = 0x100;  // AH = status
          // CF = 1 = failed
          stack->regs.flags |= 1;
          return;
        }
      }

      // AH = status = 0
      stack->regs.ax = sectors;
      // CF = 0 = success
      stack->regs.flags &= ~1;
    } break;

    case 0x08: {
      // INT 13,8 - Get Current Drive Parameters

      stack->regs.ax = 0;
      stack->regs.bx = 0x04;  // 1.44MB
      stack->regs.cx = (80 << 2) | 18;
      stack->regs.dx = (1 << 8) | 1;
      stack->regs.flags &= ~1;  // no CF

      // TODO: ES:DI = pointer to 11 byte Disk Base Table (DBT)
      // it'll be a constant thing in BIOS ROM somewhere (with SPECIFY etc
      // params)
    } break;

    default:
      puts("DISK BIOS: Unimplemented function ");
      puthex(stack->regs.ax >> 8);
      puts("\r\n");
      panic("DISK BIOS: Unimplemented function");
  }
}

void system_bios(struct isrstack __ss *stack) {
  switch (stack->regs.ax >> 8) {
    default:
      puts("SYSTEM BIOS: Unimplemented function ");
      puthex(stack->regs.ax >> 8);
      puts("\r\n");
      panic("SYSTEM BIOS: Unimplemented function");
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

#define PORT 0x3f8  // COM1

static int serialports[8] = {
    0x3F8, 0x2F8, 0x3E8, 0x2E8, 0x5F8, 0x4F8, 0x5E8, 0x4E8,
};

void configure_serial() {
  for (int i = 0; i < 8; ++i) {
    outportb(serialports[i] + 1, 0x00);  // Disable all interrupts
    outportb(serialports[i] + 3, 0x80);  // Enable DLAB (set baud rate divisor)
    outportb(serialports[i] + 0,
             0x03);  // Set divisor to 3 (lo byte) 38400 baud
    outportb(serialports[i] + 1, 0x00);  //                  (hi byte)
    outportb(serialports[i] + 3, 0x03);  // 8 bits, no parity, one stop bit
    outportb(serialports[i] + 2,
             0xC7);  // Enable FIFO, clear them, with 14-byte threshold
    outportb(serialports[i] + 4, 0x0B);  // IRQs enabled, RTS/DSR set
                                         /*
                        outportb(serialports[i] + 4,
                                 0x1E);  // Set in loopback mode, test the serial chip
                                     
                                outportb(serialports[i] + 0, 0xAE);  // Test serial chip (send byte 0xAE and
                                                                     // check if serial returns same byte)
                                     
                                // Check if serial is faulty (i.e: not same byte as sent)
                                if (inportb(serialports[i] + 0) != 0xAE) {
                                  continue;
                                }
                                */

    // If serial is not faulty set it in normal operation mode
    // (not-loopback with IRQs enabled and OUT#1 and OUT#2 bits enabled)
    outportb(serialports[i] + 4, 0x0F);

    // finally send a test character
    outportb(serialports[i], '0' + i);
  }
}

void blastall() {
  for (int i = 0; i < 8; ++i) {
    outportb(serialports[i], '0' + i);
  }
}
