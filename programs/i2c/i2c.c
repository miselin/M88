#include <conio.h> /* For inp/outp functions */
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h> /* For clock() */

/* Maximum buffer size for I2C operations */
#define MAX_BUFFER_SIZE 256

/* Timeouts in milliseconds */
#define TIMEOUT_PIN 1000 /* 1 second timeout for PIN bit */
#define TIMEOUT_BUS 2000 /* 2 second timeout for bus free */
#define TIMEOUT_ACK 500  /* 0.5 second timeout for ACK */

/* Error codes */
#define ERR_SUCCESS 0
#define ERR_TIMEOUT_PIN 1
#define ERR_TIMEOUT_BUS 2
#define ERR_TIMEOUT_ACK 3
#define ERR_NO_ACK 4
#define ERR_BUS_ERROR 5

/* Clock settings - indexed by MHz */
#define CLK_INVALID 0xFF
unsigned char get_clock_reg(int mhz) {
  switch (mhz) {
    case 3:
      return (0 << 2); /* 3 MHz */
    case 4:
      return (4 << 2); /* 4.43 MHz (we use 4 to mean 4.43) */
    case 6:
      return (5 << 2); /* 6 MHz */
    case 8:
      return (6 << 2); /* 8 MHz */
    case 12:
      return (7 << 2); /* 12 MHz */
    default:
      return CLK_INVALID;
  }
}

/* SCL frequency */
#define SCL_90K (0 << 0)  /* S21-20=00: 90 kHz */
#define SCL_45K (1 << 0)  /* S21-20=01: 45 kHz */
#define SCL_11K (2 << 0)  /* S21-20=10: 11 kHz */
#define SCL_1_5K (3 << 0) /* S21-20=11: 1.5 kHz */

/* S1 Control Register Bits */
#define S1_PIN (1 << 7) /* Pending Interrupt Not */
#define S1_ESO (1 << 6) /* Enable Serial Output */
#define S1_ENI (1 << 3) /* Enable Interrupt */
#define S1_STA (1 << 2) /* Start Condition */
#define S1_STO (1 << 1) /* Stop Condition */
#define S1_ACK (1 << 0) /* Acknowledge */

/* S1 Status Register Bits */
#define S1_BB (1 << 0)         /* Bus Busy */
#define S1_BER (1 << 4)        /* Bus Error */
#define S1_LRB (1 << 3)        /* Last Received Bit */
#define S1_PIN_STATUS (1 << 7) /* PIN status bit */

static void print_usage(const char *program_name);
static int hex_to_int(const char *hex_str);
static void init_pcf8584(unsigned port, unsigned char clk_reg,
                         unsigned char own_addr);
static int write_i2c(unsigned port, unsigned char device_addr,
                     unsigned char *data, size_t length);
static int read_i2c(unsigned port, unsigned char device_addr, size_t length);
static int check_status(unsigned port);
static int wait_for_pin(unsigned port);
static int wait_for_bus(unsigned port);
/* outp with a short bus delay per PCF8584 datasheet */
static void safe_outp(unsigned port, unsigned char value);
/* inp with a short bus delay per PCF8584 datasheet */
static unsigned char safe_inp(unsigned port);
static unsigned long get_msec(void);

int main(int argc, char *argv[]) {
  if (argc < 4) {
    print_usage(argv[0]);
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

    unsigned char clk_reg = get_clock_reg(clock_mhz);
    if (clk_reg == CLK_INVALID) {
      fprintf(
          stderr,
          "Invalid clock frequency. Must be 3, 4 (4.43), 6, 8, or 12 MHz\n");
      return 1;
    }

    init_pcf8584(port, clk_reg, addr >> 1);
    printf("PCF8584 initialized:\n");
    printf("  Clock: %d MHz\n", clock_mhz);
    printf("  Own Address: 0x%02X\n", addr);
    return 0;
  }

  unsigned char device_addr = (unsigned char)hex_to_int(argv[3]);
  if (device_addr == 0) {
    fprintf(stderr, "Invalid device address\n");
    return 1;
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

      return write_i2c(port, device_addr, buffer, data_length);
    }

    case 'r': {
      if (argc != 5) {
        fprintf(stderr, "Read function requires length parameter\n");
        return 1;
      }

      size_t length = hex_to_int(argv[4]);
      if (length == 0 || length > MAX_BUFFER_SIZE) {
        fprintf(stderr, "Invalid read length\n");
        return 1;
      }

      return read_i2c(port, device_addr, length);
    }

    case 's':
      check_status(port);
      return 0;
  }

  return 1;
}

static void init_pcf8584(unsigned port, unsigned char clk_reg,
                         unsigned char own_addr) {
  /* Initialize sequence as per datasheet section 7.1:
   * 1. Write 80h to S1 - disable serial I/O
   * 2. Write own address (not used in master mode, but required)
   * 3. Write A0h to S1 to select S2 register
   * 4. Write 1Ch to S2 - set clock to 90kHz @ 12MHz
   * 5. Write C1h to S1 - enable serial I/O, idle state
   */

  /* Set A0=1 to access S1 */
  outp(port + 1, 0x80); /* Disable serial I/O */

  /* Set A0=0 to access S0, write own address */
  outp(port, own_addr); /* Own address (shifted left internally) */

  /* Set A0=1, select S2 register */
  outp(port + 1, 0xA0);

  /* Set A0=0, write clock configuration */
  outp(port, clk_reg); /* Set clock frequency and SCL rate */

  /* Set A0=1, enable serial I/O */
  outp(port + 1, 0xC1);
}

static int write_i2c(unsigned port, unsigned char device_addr,
                     unsigned char *data, size_t length) {
  int status;

  /* Wait for bus free */
  status = wait_for_bus(port);
  if (status != ERR_SUCCESS) {
    fprintf(stderr, "Timeout waiting for bus to be free\n");
    return status;
  }

  /* Address + Write bit (0) */
  outp(port, device_addr << 1);

  /* Write control byte - generate START and send address */
  outp(port + 1, S1_ESO | S1_STA | S1_ACK | S1_PIN);

  status = wait_for_pin(port);
  if (status != ERR_SUCCESS) {
    if (status == ERR_BUS_ERROR) {
      fprintf(stderr, "Bus error during address transmission\n");
    } else {
      fprintf(stderr, "Timeout during address transmission\n");
    }
    return status;
  }

  /* Check for acknowledge */
  if (inp(port + 1) & S1_LRB) {
    fprintf(stderr, "No acknowledge from device\n");
    outp(port + 1, S1_ESO | S1_STO | S1_ACK | S1_PIN); /* Generate STOP */
    return ERR_NO_ACK;
  }

  /* Write data bytes */
  for (size_t i = 0; i < length; i++) {
    outp(port, data[i]);

    status = wait_for_pin(port);
    if (status != ERR_SUCCESS) {
      fprintf(stderr, "Timeout waiting for data transmission\n");
      outp(port + 1, S1_ESO | S1_STO | S1_ACK | S1_PIN); /* Generate STOP */
      return status;
    }

    if (inp(port + 1) & S1_LRB) {
      fprintf(stderr, "No acknowledge for data byte\n");
      outp(port + 1, S1_ESO | S1_STO | S1_ACK | S1_PIN); /* Generate STOP */
      return ERR_NO_ACK;
    }
  }

  /* Generate STOP condition */
  outp(port + 1, S1_ESO | S1_STO | S1_ACK | S1_PIN);

  return ERR_SUCCESS;
}

static int read_i2c(unsigned port, unsigned char device_addr, size_t length) {
  int status;

  /* Wait for bus free */
  status = wait_for_bus(port);
  if (status != ERR_SUCCESS) {
    fprintf(stderr, "Timeout waiting for bus to be free\n");
    return status;
  }

  /* Address + Read bit (1) */
  safe_outp(port, (device_addr << 1) | 1);

  /* Write control byte - generate START and send address */
  safe_outp(port + 1, S1_ESO | S1_STA | S1_ACK);

  status = wait_for_pin(port);
  if (status != ERR_SUCCESS) {
    if (status == ERR_BUS_ERROR) {
      fprintf(stderr, "Bus error during address transmission\n");
    } else {
      fprintf(stderr, "Timeout during address transmission\n");
    }
    safe_outp(port + 1, S1_ESO | S1_STO | S1_ACK | S1_PIN); /* Generate STOP */
    return status;
  }

  /* Check for acknowledge */
  if (safe_inp(port + 1) & S1_LRB) {
    fprintf(stderr, "No acknowledge from device\n");
    safe_outp(port + 1, S1_ESO | S1_STO | S1_ACK | S1_PIN); /* Generate STOP */
    return ERR_NO_ACK;
  }

  /* Dummy read to start reception */
  safe_inp(port);

  /* Read data bytes */
  for (size_t i = 0; i < length; i++) {
    status = wait_for_pin(port);
    if (status != ERR_SUCCESS) {
      if (status == ERR_BUS_ERROR) {
        fprintf(stderr, "Bus error during data reception\n");
      } else {
        fprintf(stderr, "Timeout during data reception\n");
      }
      safe_outp(port + 1, S1_ESO | S1_STO | S1_ACK | S1_PIN); /* Generate STOP */
      return status;
    }

    /* Check for acknowledge */
    if (safe_inp(port + 1) & S1_LRB) {
      fprintf(stderr, "No acknowledge from device\n");
      safe_outp(port + 1, S1_ESO | S1_STO | S1_ACK | S1_PIN); /* Generate STOP */
      return ERR_NO_ACK;
    }

    /* For last byte, send NAK */
    if (i == length - 1) {
      safe_outp(port + 1, S1_ESO); /* Clear ACK bit */
    }

    /* Read and output the byte */
    putchar(safe_inp(port));
  }

  status = wait_for_pin(port);
  if (status != ERR_SUCCESS) {
    if (status == ERR_BUS_ERROR) {
      fprintf(stderr, "Bus error during NACK transmission\n");
    } else {
      fprintf(stderr, "Timeout during NACK transmission\n");
    }
    safe_outp(port + 1, S1_ESO | S1_STO | S1_ACK | S1_PIN); /* Generate STOP */
    return status;
  }

  /* Generate STOP condition */
  safe_outp(port + 1, S1_ESO | S1_STO | S1_ACK | S1_PIN);
  return ERR_SUCCESS;
}

static int check_status(unsigned port) {
  unsigned char status = safe_inp(port + 1);
  printf("Status: %02X\n", status);
  printf("  PIN (Pending Interrupt Not): %d\n",
         (status & S1_PIN_STATUS) ? 1 : 0);
  printf("  BB (Bus Busy): %d\n", (status & S1_BB) ? 1 : 0);
  printf("  BER (Bus Error): %d\n", (status & S1_BER) ? 1 : 0);
  printf("  LRB (Last Received Bit): %d\n", (status & S1_LRB) ? 1 : 0);
  return ERR_SUCCESS;
}

static void bus_delay(void) {
  /* Simple delay by reading from unused port */
  for (int i = 0; i < 6; i++) {
    inp(0x80);
  }
}

static void safe_outp(unsigned port, unsigned char value) {
  outp(port, value);
  bus_delay();
}

static unsigned char safe_inp(unsigned port) {
  unsigned char value = inp(port);
  bus_delay();
  return value;
}

/* Wait for PIN bit to be cleared with timeout */
static int wait_for_pin(unsigned port) {
  unsigned long start = get_msec();

  while (1) {
    unsigned char status = safe_inp(port + 1);

    /* Check for bus error */
    if (status & S1_BER) {
      return ERR_BUS_ERROR;
    }

    /* Check if PIN cleared */
    if ((status & S1_PIN_STATUS) == 0) {
      return ERR_SUCCESS;
    }

    if (get_msec() - start > TIMEOUT_PIN) {
      return ERR_TIMEOUT_PIN;
    }
  }
}

/* Wait for bus to be free with timeout */
static int wait_for_bus(unsigned port) {
  unsigned long start = get_msec();

  while ((inp(port + 1) & S1_BB) == 0) { /* BB=0 means busy */
    if (get_msec() - start > TIMEOUT_BUS) {
      return ERR_TIMEOUT_BUS;
    }
  }
  return ERR_SUCCESS;
}

static void print_usage(const char *program_name) {
  fprintf(stderr, "Usage:\n");
  fprintf(stderr,
          "  %s <port> w <device> <data>...  - Write data to I2C device\n",
          program_name);
  fprintf(stderr,
          "  %s <port> r <device> <length>   - Read data from I2C device\n",
          program_name);
  fprintf(stderr, "  %s <port> s <device>            - Check device status\n",
          program_name);
  fprintf(stderr, "  %s <port> i [clock] [addr]      - Initialize PCF8584\n",
          program_name);
  fprintf(stderr,
          "       clock: Clock frequency in MHz (default: 4 for 4.43MHz)\n");
  fprintf(stderr, "              Valid values: 3, 4 (4.43), 6, 8, 12\n");
  fprintf(stderr, "       addr:  Own address in hex (default: 0xAA)\n");
  fprintf(stderr, "All numbers are hexadecimal\n");
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

static unsigned long get_msec(void) {
  return (unsigned long)(clock() * 1000 / CLOCKS_PER_SEC);
}
