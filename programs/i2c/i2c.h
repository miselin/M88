// I2C library routines for controlling a PCF8584 on the ISA Bus
// Designed for use with the M88 ISA-to-I2C card.
// Copyright 2024 Matthew Iselin, see github.com/miselin/m88 for license.

#ifndef _I2C_H
#define _I2C_H

#include <stdint.h>
#include <stdlib.h>

/* Timeouts in milliseconds */
#define TIMEOUT_PIN 1000 /* 1 second timeout for PIN bit */
#define TIMEOUT_BUS 2000 /* 2 second timeout for bus free */
#define TIMEOUT_ACK 500  /* 0.5 second timeout for ACK */

/* Error codes */
#define ERR_SUCCESS 0
#define ERR_TIMEOUT_PIN 1
#define ERR_TIMEOUT_BUS 2
#define ERR_NO_ACK 4
#define ERR_BUS_ERROR 5

#define CLK_INVALID 0xFF

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

union pcf8584_status {
  struct {
    char bb : 1;
    char lab : 1;
    char aas : 1;
    char ad0_lrb : 1;
    char ber : 1;
    char sts : 1;
    char rsvd : 1;
    char pin : 1;
  } bits;

  uint8_t raw;
};

void pcf8584_init(unsigned port, unsigned char clk_reg, unsigned char own_addr);
int pcf8584_write(unsigned port, unsigned char device_addr, unsigned char *data,
                  size_t length);
int pcf8584_read(unsigned port, unsigned char device_addr, unsigned char *data,
                 size_t length);
int pcf8584_status(unsigned port, union pcf8584_status *out);
int pcf8584_wait_for_pin(unsigned port);
int pcf8584_wait_for_bus(unsigned port);

unsigned char pcf8584_get_clock_reg_for(int mhz);

const char *pcf8584_strerror(int error);

#endif
