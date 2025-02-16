#include "i2c.h"

#include <conio.h> /* For inp/outp functions */
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h> /* For clock() */

#define VERSION "1.0"

/* Maximum buffer size for I2C operations */
#define MAX_BUFFER_SIZE 256

static void print_usage();
static int hex_to_int(const char *hex_str);
static int check_status_command(unsigned port);

int main(int argc, char *argv[]) {
  // Minimum argv is "i2c <PORT> <FUNCTION>"
  if (argc < 3) {
    print_usage();
    return 1;
  }

  /* Parse I/O port */
  unsigned port = hex_to_int(argv[1]);
  if (port == 0) {
    fprintf(stderr, "Invalid port number\n");
    return 1;
  }

  /* Parse function */
  char function = tolower(argv[2][0]);
  if (function != 'r' && function != 'w' && function != 's' &&
      function != 'i') {
    fprintf(stderr, "Invalid function (must be r, w, s, or i)\n");
    return 1;
  }

  if (function == 'i') {
    int clock_mhz = 4;         /* Default: 4 (4.43MHz) */
    unsigned char addr = 0xAA; /* Default: 0xAA */

    if (argc >= 4) {
      clock_mhz = atoi(argv[3]); /* Take clock frequency as decimal MHz */
    }
    if (argc >= 5) {
      addr = (unsigned char)hex_to_int(argv[4]); /* Take address as hex */
    }

    unsigned char clk_reg = pcf8584_get_clock_reg_for(clock_mhz);
    if (clk_reg == CLK_INVALID) {
      fprintf(
          stderr,
          "Invalid clock frequency. Must be 3, 4 (4.43), 6, 8, or 12 MHz\n");
      return 1;
    }

    pcf8584_init(port, clk_reg, addr >> 1);
    printf("PCF8584 initialized:\n");
    printf("  Clock: %d MHz\n", clock_mhz);
    printf("  Own Address: 0x%02X\n", addr);
    return 0;
  }

  unsigned char device_addr = 0;
  if (function == 'w' || function == 'r') {
    if (argc < 4) {
      fprintf(stderr, "A target I2C device address is required.\n");
      return 1;
    }

    device_addr = (unsigned char)hex_to_int(argv[3]);
    if (device_addr == 0) {
      fprintf(stderr, "Invalid device address\n");
      return 1;
    }
  }

  switch (function) {
    case 'w': {
      if (argc < 5) {
        fprintf(stderr, "Write function requires at least one data byte\n");
        return 1;
      }

      unsigned char buffer[MAX_BUFFER_SIZE];
      size_t data_length = 0;

      /* Parse all remaining arguments as hex data */
      for (int i = 4; i < argc && data_length < MAX_BUFFER_SIZE; i++) {
        buffer[data_length] = (unsigned char)hex_to_int(argv[i]);
        if (buffer[data_length] == 0 && argv[i][0] != '0') {
          fprintf(stderr, "Invalid data byte: %s\n", argv[i]);
          return 1;
        }
        data_length++;
      }

      return pcf8584_write(port, device_addr, buffer, data_length);
    }

    case 'r': {
      if (argc != 5) {
        fprintf(stderr, "Read function requires length parameter\n");
        return 1;
      }

      size_t length = atoi(argv[4]);
      if (length == 0 || length > MAX_BUFFER_SIZE) {
        fprintf(stderr, "Invalid read length\n");
        return 1;
      }

      unsigned char buffer[MAX_BUFFER_SIZE];

      int result = pcf8584_read(port, device_addr, buffer, length);
      if (result != ERR_SUCCESS) {
        return result;
      }

      for (size_t i = 0; i < length; ++i) {
        putchar(buffer[i]);
      }
    }

    case 's':
      check_status_command(port);
      return 0;
  }

  return 1;
}

static int check_status_command(unsigned port) {
  union pcf8584_status status;
  int result = pcf8584_status(port, &status);
  if (result != ERR_SUCCESS) {
    fprintf(stderr, "Failed to check status\n");
    return result;
  }

  printf("Status: %02X\n", status.raw);
  printf("  PIN (Pending Interrupt Not): %d\n", status.bits.pin);
  printf("  BB (Bus Busy): %d\n", status.bits.bb);
  printf("  BER (Bus Error): %d\n", status.bits.ber);
  printf("  LRB (Last Received Bit): %d\n", status.bits.ad0_lrb);
  return ERR_SUCCESS;
}

static void print_usage() {
  fprintf(stderr, "M88 ISA-to-I2C Utility %s\n\n", VERSION);
  fprintf(stderr, "Usage:\n");
  fprintf(stderr,
          "  I2C <port> w <device> <byte>...  - Write data to I2C device, with "
          "spaces\n");
  fprintf(stderr,
          "                                     between each byte. For "
          "example, 01 AB FE\n");
  fprintf(stderr,
          "  I2C <port> r <device> <length>   - Read data from I2C device\n");
  fprintf(stderr, "  I2C <port> s                     - Check device status\n");
  fprintf(stderr, "  I2C <port> i [clock] [addr]      - Initialize PCF8584\n");
  fprintf(stderr,
          "       clock: Clock frequency in MHz (default: 4 for 4.43MHz)\n");
  fprintf(stderr, "              Valid values: 3, 4 (4.43), 6, 8, 12\n");
  fprintf(stderr, "       addr:  Own address in hex (default: AA)\n");
  fprintf(stderr, "\nPort and device addresses are parsed as hexadecimal.\n");
}

static int hex_to_int(const char *hex_str) {
  int result = 0;
  while (*hex_str) {
    char c = tolower(*hex_str++);
    if (c >= '0' && c <= '9')
      result = (result << 4) | (c - '0');
    else if (c >= 'a' && c <= 'f')
      result = (result << 4) | (c - 'a' + 10);
    else
      return 0; /* Invalid hex character */
  }
  return result;
}
