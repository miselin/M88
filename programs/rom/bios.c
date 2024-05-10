#include <dos.h>

#include "bda.h"
#include "cpu.h"
#include "string.h"

struct bda_t __far *bda = MK_FP(0x0000, 0x0000);

void disk_bios(struct isrstack __ss *stack);
void system_bios(struct isrstack __ss *stack);
void keyboard_bios(struct isrstack __ss *stack);
void serial_bios(struct isrstack __ss *stack);
void printer_bios(struct isrstack __ss *stack);
void rtc_bios(struct isrstack __ss *stack);

struct sysconfig_t {
  uint16_t length;
  uint8_t model;
  uint8_t submodel;
  uint8_t biosrev;
  union {
    struct {
      uint8_t rsvd : 1;
      uint8_t microchannel : 1;
      uint8_t ebda : 1;
      uint8_t wait : 1;
      uint8_t int15_4f : 1;
      uint8_t rtc : 1;
      uint8_t two_pics : 1;
      uint8_t dma3_disk : 1;
    };
    uint8_t flags;
  };
  uint32_t reserved;
} sysconfig = {
    .length = sizeof(sysconfig),
    .model = 0xFE,
    .submodel = 0x00,
    .biosrev = 0x01,
    .flags = 1 << 6,  // two PICs
    .reserved = 0x00000000,
};

extern uint16_t isr_array[256];

static int bios_isrs[] = {0x11, 0x12, 0x14, 0x15, 0x16, 0x17, 0x1A};

inline uint16_t getcs() {
  uint16_t val;
  asm {
    mov val, cs
  }
  return val;
}

void install_bios_isrs(uint16_t cs) {
  for (int i = 0; i < sizeof(bios_isrs) / sizeof(bios_isrs[0]); ++i) {
    int isrnum = bios_isrs[i];
    bda->ivt[isrnum].segment = cs;
    bda->ivt[isrnum].offset = isr_array[isrnum];

    puts("Installed BIOS ISR ");
    puthex(isrnum);
    puts(" at ");
    puthex(cs);
    puts(":");
    puthex(isr_array[isrnum]);
    puts("\r\n");
  }
}

int handle_bios_interrupt(struct isrstack __ss *stack, unsigned short isrnum) {
  switch (isrnum) {
    case 0x10:
      //  INT 10 - Video BIOS Services
      //  no-op, should be installed by option ROMs
      return 0;
    case 0x11:
      // INT 11 - BIOS Equipment Determination
      stack->regs.ax = bda->equipflags_raw;
      return 0;
    case 0x12:
      // INT 12 - Memory Size Determination
      stack->regs.ax = bda->memsize;
      return 0;
    case 0x13:
      disk_bios(stack);
      return 0;
    case 0x14:
      serial_bios(stack);
      return 0;
    case 0x15:
      system_bios(stack);
      return 0;
    case 0x16:
      keyboard_bios(stack);
      return 0;
    case 0x17:
      printer_bios(stack);
      return 0;
    case 0x19:
      // warm reboot
      panic("INT 19\r\n");
      break;
    case 0x1A:
      rtc_bios(stack);
      return 0;
  }

  return 1;
}

void disk_bios(struct isrstack __ss *stack) {
  serial_puts("DISK BIOS: ");
  serial_puthex(stack->regs.ax >> 8);
  serial_puts("\r\n");

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

    case 0x15: {
      uint8_t drive = stack->regs.dx & 0xFF;
      puts("DISK BIOS: get drive type ");
      puthex(drive);
      puts("\r\n");

      stack->regs.flags &= ~F_CF;

      switch (drive) {
        case 0x00:
          stack->regs.ax = 0x0100;  // AH = 1 (diskette, no change detection)
          break;
        case 0x01:
        case 0x81:
          stack->regs.ax = 0x0000;  // AH = 0 (drive not present)
          break;
        case 0x80:
          stack->regs.ax = 0x0000;  // fake no drive for now
          // stack->regs.ax = 0x0300;  // AH = 3 (fixed disk present)
          break;
        default:
          stack->regs.flags |= F_CF;  // unknown drive
          break;
      }
    } break;

    default:
      puts("DISK BIOS: Unimplemented function ");
      puthex(stack->regs.ax >> 8);
      puts("\r\n");
      panic("DISK BIOS: Unimplemented function");
  }
}

void serial_bios(struct isrstack __ss *stack) {
  serial_puts("SERIAL BIOS: ");
  serial_puthex(stack->regs.ax >> 8);
  serial_puts("\r\n");

  switch (stack->regs.ax >> 8) {
    case 0x00:
      puts("SERIAL BIOS: Initialize Serial Port\r\n");
      puthex(stack->regs.ax & 0xFF);
      puts("  ");
      puthex(stack->regs.dx);
      puts("\r\n");
      stack->regs.ax = 0;
      break;
    case 0x01:
      // INT 14,1 - Send One Character
      // TODO
      stack->regs.ax &= 0xFF;
      stack->regs.flags &= ~F_CF;
      break;
    case 0x02:
      // INT 14,2 - Receive One Character
      // TODO
      stack->regs.ax &= 0xFF;
      stack->regs.flags &= ~F_CF;
      break;
    case 0x03:
      // INT 14,3 - Get Port Status
      // TODO
      stack->regs.ax = 0;
      stack->regs.flags &= ~F_CF;
      break;
    default:
      puts("SERIAL BIOS: Unimplemented function ");
      puthex(stack->regs.ax >> 8);
      puts("\r\n");
      panic("SERIAL BIOS: Unimplemented function");
  }
}

void system_bios(struct isrstack __ss *stack) {
  serial_puts("SYSTEM BIOS: ");
  serial_puthex(stack->regs.ax >> 8);
  serial_puts("\r\n");

  switch (stack->regs.ax >> 8) {
    case 0xC0:
      stack->regs.es = getcs();
      stack->regs.bx = (uint16_t)&sysconfig;
      puts("SYSTEM BIOS: Get System Configuration\r\n");
      puthex(stack->regs.es);
      puts(":");
      puthex(stack->regs.bx);
      puts("\r\n");
      stack->regs.flags &= ~1;  // CF = 0
      break;

    default:
      puts("SYSTEM BIOS: Unimplemented function ");
      puthex(stack->regs.ax >> 8);
      puts("\r\n");
      puthex(0xbeef);

      // CF is set but AX is left unchanged if the function is not supported
      stack->regs.flags |= F_CF;
  }
}

void keyboard_bios(struct isrstack __ss *stack) {
  serial_puts("KEYBOARD BIOS: ");
  serial_puthex(stack->regs.ax >> 8);
  serial_puts("\r\n");

  switch (stack->regs.ax >> 8) {
    case 0x00:
      // INT 16,0 - Read Next Character
      // TODO
      break;

    case 0x01:
      // INT 16,1 - Report If Character Ready
      // TODO - note returns character but if ready but does not remove from
      // buffer
      stack->regs.flags |= F_ZF;
      break;

    case 0x02:
      // INT 16,2 - Get Shift Status
      // TODO
      stack->regs.ax = 0;
      break;

    default:
      puts("SYSTEM BIOS: Unimplemented function ");
      puthex(stack->regs.ax >> 8);
      puts("\r\n");
      puthex(0xbeef);

      // CF is set but AX is left unchanged if the function is not supported
      stack->regs.flags |= F_CF;
  }
}

void printer_bios(struct isrstack __ss *stack) {
  serial_puts("PRINTER BIOS: ");
  serial_puthex(stack->regs.ax >> 8);
  serial_puts("\r\n");

  switch (stack->regs.ax >> 8) {
    case 0x01:
      puts("PRINTER BIOS: Initialize Printer Port\r\n");
      puthex(stack->regs.dx);
      puts("\r\n");
      stack->regs.ax = 0;
      break;
    default:
      puts("PRINTER BIOS: Unimplemented function ");
      puthex(stack->regs.ax >> 8);
      puts("\r\n");
      panic("PRINTER BIOS: Unimplemented function");
  }
}

void rtc_bios(struct isrstack __ss *stack) {
  serial_puts("RTC BIOS: ");
  serial_puthex(stack->regs.ax >> 8);
  serial_puts("\r\n");

  switch (stack->regs.ax >> 8) {
    case 0x00:
      // INT 1A,0 - Read System Clock Counter
      stack->regs.ax = bda->clockrollover & 0xFF;
      stack->regs.dx = bda->timer & 0xFFFF;
      stack->regs.cx = bda->timer >> 16;
      break;

    /*
    case 0x01:
      puts("RTC BIOS: Initialize Printer Port\r\n");
      puthex(stack->regs.dx);
      puts("\r\n");
      stack->regs.ax = 0;
      break;
      */
    case 0x02:
      // no RTC
      stack->regs.flags |= F_CF;
      break;
    default:
      puts("RTC BIOS: Unimplemented function ");
      puthex(stack->regs.ax >> 8);
      puts("\r\n");
      panic("RTC BIOS: Unimplemented function");
  }
}
