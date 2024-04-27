#include <dos.h>

char __far *bda = MK_FP(0x40, 0x00);

#pragma pack(1)

struct optionrom_hdr {
  unsigned short signature;
  unsigned char length;  // in 512-byte blocks
};

struct ivt_entry {
  unsigned short offset;
  unsigned short segment;
};

struct ivt_entry __far *ivt = MK_FP(0x00, 0x00);

#pragma pack()

typedef void(__far *optionrom_entry_t)();

extern unsigned short isr_array[256];

void outportb(unsigned short port, unsigned char value) {
  asm {
    mov dx, port
    mov al, value
    out dx, al
  }
}

unsigned short inportb(unsigned short port) {
  unsigned short value;
  asm {
    xor ax, ax
    mov dx, port
    in al, dx
  }
}

void update_7seg(unsigned char value) { outportb(0xF0, value); }

void write_bochs_char(char c) { outportb(0xE9, c); }

void write_bochs(const char *s) {
  while (*s) {
    write_bochs_char(*s);
    ++s;
  }
}

void write_bochs_ss(const char __ss *s) {
  while (*s) {
    write_bochs_char(*s);
    ++s;
  }
}

void panic(const char *s) {
  write_bochs("PANIC: ");
  write_bochs(s);
  write_bochs("\n");
  asm { cli; hlt }
}

void configure_pic();
void configure_pit();
void configure_kbc();
extern call_option_roms();

extern load_ivt();

int rommain() {
  load_ivt();

  update_7seg(0x03);

  configure_pic();

  update_7seg(0x04);

  configure_pit();

  update_7seg(0x05);

  configure_kbc();

  update_7seg(0x06);

  asm { sti }

  update_7seg(0x07);

  call_option_roms();

  update_7seg(0x08);

  // configure FDC

  // read boot sector, run it

  while (1)
    ;

  return 0;
}

void irqhandler(unsigned short irqnum) {
  if (irqnum == 0) {
    // timer
    write_bochs_char('T');
  } else if (irqnum == 6) {
    // floppy
  }

  outportb(0x20, 0x20);
}

void isrhandler(unsigned short isrnum) {
  if (isrnum >= 8 && isrnum <= 15) {
    irqhandler(isrnum - 8);
    return;
  }
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

void configure_pit() {
  // Channel 0 - lo/hi byte, rate generator, binary mode
  outportb(0x43, 0x36);

  // 18.2 Hz
  outportb(0x40, 0xFF);
  outportb(0x40, 0xFF);

  // unmask timer interrupt
  outportb(0x21, 0xFE);

  // set BDA counters etc to 0...
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
    panic("KBC self-test failed\n");
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
