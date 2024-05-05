#include "fdc.h"

#include <dos.h>

#include "bda.h"
#include "pit.h"
#include "string.h"

#define FDC_STATUS_REGISTER_A 0x3F0
#define FDC_STATUS_REGISTER_B 0x3F1
#define FDC_DIGITAL_OUTPUT_REGISTER 0x3F2
#define FDC_TAPE_DRIVE_REGISTER 0x3F3
#define FDC_MAIN_STATUS_REGISTER 0x3F4      // when read
#define FDC_DATARATE_SELECT_REGISTER 0x3F4  // when written
#define FDC_DATA_FIFO 0x3F5
#define FDC_DIGITAL_INPUT_REGISTER 0x3F7
#define FDC_CONFIG_CONTROL_REGISTER 0x3F7

struct bda_t __far *bda = MK_FP(0x0000, 0x0000);

int fdc_sense_interrupt(int device, int expected);

void prepare_dma2(uint16_t seg, uint16_t addr, uint16_t length);
void prepare_dma2_read(uint16_t seg, uint16_t addr, uint16_t length);

int fdc_reset() {
  outportb(FDC_DIGITAL_OUTPUT_REGISTER, 0x00);
  delay_ticks(1);
  outportb(FDC_DIGITAL_OUTPUT_REGISTER, 0x0C);
  delay_ticks(2);

  int ok = 0;
  for (int i = 0; i < 10; ++i) {
    uint8_t status = inportb(FDC_MAIN_STATUS_REGISTER);
    if ((status & 0xC0) == 0x80) {
      ok = 1;
      break;
    }

    delay_ticks(1);
  }

  if (!ok) {
    return 1;
  }

  // sense interrupt, once per potential drive (4x)
  for (int i = 0; i < 4; ++i) {
    if (fdc_sense_interrupt(i, 0xC0 | i)) {
      puts("FDC sense interrupt failed\n");
      return 1;
    }
  }

  // set default data rate, 500 kbps
  outportb(FDC_CONFIG_CONTROL_REGISTER, 0x00);

  return 0;
}

int fdc_wait_rqm() {
  for (int i = 0; i < 10; ++i) {
    uint8_t status = inportb(FDC_MAIN_STATUS_REGISTER);
    if ((status & 0x80) == 0x80) {
      return 0;
    }

    delay_ticks(1);
  }

  return 1;
}

int fdc_start_command(uint8_t command) {
  int ok = 0;
  for (int i = 0; i < 3; ++i) {
    if (!fdc_wait_rqm()) {
      ok = 1;
      break;
    }
  }

  if (!ok) {
    return 1;
  }

  outportb(FDC_DATA_FIFO, command);

  return 0;
}

int fdc_send_parameter(uint8_t param) {
  int ready = 0;
  for (int i = 0; i < 3; ++i) {
    if (fdc_wait_rqm()) {
      return 1;
    }

    // check DIO=0 (FDC expects write)
    uint8_t status = inportb(FDC_MAIN_STATUS_REGISTER);
    if ((status & 0x40) == 0) {
      ready = 1;
      break;
    }
  }

  if (!ready) {
    return 1;
  }

  outportb(FDC_DATA_FIFO, param);

  return 0;
}

// Wait for the FDC FIFO to have data ready to read
int fdc_fifo_read_wait() {
  int ready = 0;
  for (int i = 0; i < 3; ++i) {
    if (fdc_wait_rqm()) {
      return 1;
    }

    // check DIO=1 (FDC expects read)
    uint8_t status = inportb(FDC_MAIN_STATUS_REGISTER);
    if ((status & 0x40) == 0x40) {
      ready = 1;
      break;
    }

    delay_ticks(1);
  }

  if (!ready) {
    return 1;
  }

  return 0;
}

// Wait for the FDC FIFO to be ready for a write
int fdc_fifo_write_wait() {
  int ready = 0;
  for (int i = 0; i < 3; ++i) {
    if (fdc_wait_rqm()) {
      return 1;
    }

    // check DIO=0 (FDC expects write)
    uint8_t status = inportb(FDC_MAIN_STATUS_REGISTER);
    if ((status & 0x40) == 0) {
      ready = 1;
      break;
    }

    delay_ticks(1);
  }

  if (!ready) {
    return 1;
  }

  return 0;
}

// Wait for a floppy IRQ
// Should be called with IRQs disabled, the function will enable them
// You should disable IRQs before sending the command that causes an IRQ
int fdc_wait_irq(uint16_t maxticks) {
  bda->recal.intflag = 0;

  asm { sti }

  // wait for this many ticks for the IRQ
  for (uint16_t i = 0; i < maxticks; ++i) {
    if (bda->recal.intflag) {
      return 0;
    }

    delay_ticks(1);
  }

  return 1;
}

// Wait for the FDC to complete running a command
int fdc_wait_complete() {
  for (int i = 0; i < 10; ++i) {
    if (fdc_wait_rqm()) {
      return 1;
    }

    uint8_t status = inportb(FDC_MAIN_STATUS_REGISTER);
    if ((status & 0x10) == 0) {
      return 0;
    }

    delay_ticks(4);
  }

  return 1;
}

// Read a byte from the FDC FIFO
int fdc_fifo_read(uint8_t __ss *data) {
  if (fdc_fifo_read_wait()) {
    return 1;
  }

  *data = inportb(FDC_DATA_FIFO);

  return 0;
}

// Write a byte to the FDC FIFO
int fdc_fifo_write(uint8_t data) {
  if (fdc_fifo_write_wait()) {
    return 1;
  }

  outportb(FDC_DATA_FIFO, data);

  return 0;
}

int fdc_sense_interrupt(int device, int expected) {
  if (fdc_start_command(0x08)) {
    puts("sense interrupt failed: start\n");
    return 1;
  }

  if (fdc_wait_rqm()) {
    puts("sense interrupt failed: waiting for completion\n");
    return 1;
  }

  uint8_t status = 0;

  // st0
  if (fdc_fifo_read(&status)) {
    puts("sense interrupt failed: st0\n");
    return 1;
  }

  if (status != expected) {
    puts("sense interrupt failed: st0 not ");
    puthex(expected);
    puts("\n");
    return 1;
  }

  // cyl
  if (fdc_fifo_read(&status)) {
    puts("sense interrupt failed: cyl\n");
    return 1;
  }

  puts("sense interrupt done\n");

  return 0;
}

int configure_fdc() {
  // allow IRQ6
  uint8_t mask = inportb(0x21);
  mask &= 0xBF;
  outportb(0x21, mask);

  if (fdc_reset()) {
    puts("FDC reset failed\r\n");
    return 1;
  }

  // Version
  uint8_t version = 0;
  if (fdc_start_command(0x10)) {
    puts("FDC failed to start Version command\r\n");
    return 1;
  }
  fdc_fifo_read(&version);

  if (version != 0x90) {
    puts("FDC is not an enhanced controller\r\n");
    return 1;
  }

  // SPECIFY

  if (fdc_start_command(0x13)) {
    puts("FDC failed to start Specify command\r\n");
    return 1;
  }

  fdc_send_parameter(0x00);
  fdc_send_parameter(0x57);  // implied seek, enable FIFO, polling mode off, 8
                             // byte FIFO threshold
  fdc_send_parameter(0x00);

  fdc_wait_rqm();

  // CONFIGURE

  if (fdc_start_command(0x03)) {
    puts("FDC failed to start Configure command\r\n");
    return 1;
  }

  fdc_send_parameter(0x80);  // SRT = 8, HUT = 0
  fdc_send_parameter(0x0A);  // HLT = 5, ND = 0 (enable DMA)

  fdc_wait_rqm();

  fdc_motor_on();
  delay_ticks(9);  // let the motor spin up

  fdc_wait_rqm();

  // RECALIBRATE

  asm { cli }

  if (fdc_start_command(0x07)) {
    puts("FDC failed to start Recalibrate command\r\n");
    return 1;
  }

  fdc_send_parameter(0x00);

  if (fdc_wait_irq(36)) {
    puts("FDC RECALIBRATE did not complete in time\r\n");
    return 1;
  }

  if (fdc_sense_interrupt(0, 0x20)) {
    puts("FDC failed to run SENSE INTERRUPT after RECALIBRATE\r\n");
    return 1;
  }

  return 0;
}

void fdc_irq() { bda->recal.intflag = 1; }

int fdc_motor_off() {
  if (fdc_wait_rqm()) {
    return 1;
  }

  outportb(FDC_DIGITAL_OUTPUT_REGISTER, 0x0C);
  bda->motorshutoff = 0;
  bda->motor.motor0 = 0;
}

int fdc_motor_on() {
  if (fdc_wait_rqm()) {
    return 1;
  }

  outportb(FDC_DIGITAL_OUTPUT_REGISTER, 0x1C);
  bda->motorshutoff = 36;  // 2 second shutoff
  bda->motor.motor0 = 1;
}

int fdc_read_drive0(uint8_t head, uint8_t cyl, uint8_t sector, uint16_t seg,
                    uint16_t addr, uint16_t length) {
  prepare_dma2_read(seg, addr, length);

  // avoid motor shutoff during read
  bda->motorshutoff = 0xFF;

  // new operation starting, clear disk operation status
  bda->diskstatus_raw = 0;

  if (length != 512) {
    puts("FDC read: only 512 byte sectors supported\r\n");
    bda->diskstatus.invalid = 1;
    return 1;
  }

  if (fdc_start_command(0xE6)) {
    return 1;
  }

  fdc_send_parameter(head << 2);  // HDS, drive select
  fdc_send_parameter(cyl);        // cylinder
  fdc_send_parameter(head);       // head
  fdc_send_parameter(sector);     // sector
  fdc_send_parameter(2);          // size (128 << 2)
  fdc_send_parameter(sector);     // EOT = 1 sector
  fdc_send_parameter(0x1B);       // GPL: GAP1 default size
  fdc_send_parameter(0xFF);       // DTL: 512 byte sectors, no special size

  if (fdc_wait_irq(36)) {
    puts("FDC read: wait timeout during read");
    bda->diskstatus.timeout = 1;
    return 1;
  }

  uint8_t result = 0;

  // st0
  if (fdc_fifo_read(&result)) {
    puts("FDC read: failed to read ST0");
    return 1;
  }

  // st1
  if (fdc_fifo_read(&result)) {
    puts("FDC read: failed to read ST1");
    return 1;
  }

  // st2
  if (fdc_fifo_read(&result)) {
    puts("FDC read: failed to read ST2");
    return 1;
  }

  // cyl
  if (fdc_fifo_read(&result)) {
    puts("FDC read: failed to read CYL");
    return 1;
  }

  // last head
  if (fdc_fifo_read(&result)) {
    puts("FDC read: failed to read LH");
    return 1;
  }

  // last sector
  if (fdc_fifo_read(&result)) {
    puts("FDC read: failed to read LS");
    return 1;
  }

  // always 2
  if (fdc_fifo_read(&result)) {
    puts("FDC read: failed to read 2");
    return 1;
  }

  // 2 second motor shutoff after read (assuming no further reads)
  bda->motorshutoff = 36;

  // success
  bda->diskstatus_raw = 0;

  return 0;
}

void prepare_dma2(uint16_t seg, uint16_t addr, uint16_t length) {
  // seg:addr -> linear
  uint32_t linearaddr = ((uint32_t)seg << 4) + addr;

  outportb(0x0A, 0x06);                      // mask DMA2
  outportb(0x0C, 0x00);                      // clear byte pointer flip-flop
  outportb(0x04, linearaddr & 0xFF);         // address low byte
  outportb(0x04, (linearaddr >> 8) & 0xFF);  // address high byte
  outportb(0x0C, 0x00);                      // clear byte pointer flip-flop
  outportb(0x05, (length - 1) & 0xFF);       // count low byte
  outportb(0x05, (length - 1) >> 8);         // count high byte
  outportb(0x81, linearaddr >> 16);          // A16-A19
  outportb(0x0A, 0x02);                      // unmask DMA2
}

void prepare_dma2_read(uint16_t seg, uint16_t addr, uint16_t length) {
  prepare_dma2(seg, addr, length);

  outportb(0x0A, 0x06);  // mask during configuration
  outportb(0x0B, 0x46);
  outportb(0x0A, 0x02);  // unmask
}
