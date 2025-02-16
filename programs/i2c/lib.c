#include <conio.h> /* For inp/outp functions */
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h> /* For clock() */

#include "i2c.h"

/* Clock settings - indexed by MHz */
unsigned char pcf8584_get_clock_reg_for(int mhz) {
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

/* outp with a short bus delay per PCF8584 datasheet */
static void safe_outp(unsigned port, unsigned char value);
/* inp with a short bus delay per PCF8584 datasheet */
static unsigned char safe_inp(unsigned port);
static unsigned long get_msec(void);

void pcf8584_init(unsigned port, unsigned char clk_reg,
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

int pcf8584_write(unsigned port, unsigned char device_addr, unsigned char *data,
                  size_t length) {
  int status;

  /* Wait for bus free */
  status = pcf8584_wait_for_bus(port);
  if (status != ERR_SUCCESS) {
    fprintf(stderr, "Timeout waiting for bus to be free\n");
    return status;
  }

  /* Address + Write bit (0) */
  outp(port, device_addr << 1);

  /* Write control byte - generate START and send address */
  outp(port + 1, S1_ESO | S1_STA | S1_ACK | S1_PIN);

  status = pcf8584_wait_for_pin(port);
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

    status = pcf8584_wait_for_pin(port);
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

int pcf8584_read(unsigned port, unsigned char device_addr, unsigned char *data,
                 size_t length) {
  int status;

  /* Wait for bus free */
  status = pcf8584_wait_for_bus(port);
  if (status != ERR_SUCCESS) {
    fprintf(stderr, "Timeout waiting for bus to be free\n");
    return status;
  }

  /* Address + Read bit (1) */
  safe_outp(port, (device_addr << 1) | 1);

  /* Write control byte - generate START and send address */
  safe_outp(port + 1, S1_ESO | S1_STA | S1_ACK);

  status = pcf8584_wait_for_pin(port);
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
    status = pcf8584_wait_for_pin(port);
    if (status != ERR_SUCCESS) {
      if (status == ERR_BUS_ERROR) {
        fprintf(stderr, "Bus error during data reception\n");
      } else {
        fprintf(stderr, "Timeout during data reception\n");
      }
      safe_outp(port + 1,
                S1_ESO | S1_STO | S1_ACK | S1_PIN); /* Generate STOP */
      return status;
    }

    /* Check for acknowledge */
    if (safe_inp(port + 1) & S1_LRB) {
      fprintf(stderr, "No acknowledge from device\n");
      safe_outp(port + 1,
                S1_ESO | S1_STO | S1_ACK | S1_PIN); /* Generate STOP */
      return ERR_NO_ACK;
    }

    /* For last byte, send NAK */
    if (i == length - 1) {
      safe_outp(port + 1, S1_ESO); /* Clear ACK bit */
    }

    /* Read and output the byte */
    *data++ = safe_inp(port);
  }

  status = pcf8584_wait_for_pin(port);
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

int pcf8584_status(unsigned port, union pcf8584_status *out) {
  unsigned char status = safe_inp(port + 1);
  out->raw = status;
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
int pcf8584_wait_for_pin(unsigned port) {
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
int pcf8584_wait_for_bus(unsigned port) {
  unsigned long start = get_msec();

  while ((inp(port + 1) & S1_BB) == 0) { /* BB=0 means busy */
    if (get_msec() - start > TIMEOUT_BUS) {
      return ERR_TIMEOUT_BUS;
    }
  }
  return ERR_SUCCESS;
}

static unsigned long get_msec(void) {
  return (unsigned long)(clock() * 1000 / CLOCKS_PER_SEC);
}
