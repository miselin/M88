#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <conio.h>

/* Maximum buffer size for I2C operations */
#define MAX_BUFFER_SIZE 256

/* S1 register (Read - Status) */
#define PCF8584_BIT_STS_BB  0
#define PCF8584_BIT_STS_LAB 1
#define PCF8584_BIT_STS_AAS 2
#define PCF8584_BIT_STS_LRB 3
#define PCF8584_BIT_STS_BER 4
#define PCF8584_BIT_STS_STS 5
#define PCF8584_BIT_STS_PIN 7

#define PCF8584_MASK_STS_BB  (1 << PCF8584_BIT_STS_BB)
#define PCF8584_MASK_STS_LAB (1 << PCF8584_BIT_STS_LAB)
#define PCF8584_MASK_STS_AAS (1 << PCF8584_BIT_STS_AAS)
#define PCF8584_MASK_STS_LRB (1 << PCF8584_BIT_STS_LRB)
#define PCF8584_MASK_STS_BER (1 << PCF8584_BIT_STS_BER)
#define PCF8584_MASK_STS_STS (1 << PCF8584_BIT_STS_STS)
#define PCF8584_MASK_STS_PIN (1 << PCF8584_BIT_STS_PIN)

/* S1 register (Write - Control) */
#define PCF8584_BIT_CTL_ACK 0
#define PCF8584_BIT_CTL_STO 1
#define PCF8584_BIT_CTL_STA 2
#define PCF8584_BIT_CTL_ENI 3
#define PCF8584_BIT_CTL_ES  4
#define PCF8584_BIT_CTL_ESO 6
#define PCF8584_BIT_CTL_PIN 7

#define PCF8584_MASK_CTL_ACK  (1 << PCF8584_BIT_CTL_ACK)
#define PCF8584_MASK_CTL_STO  (1 << PCF8584_BIT_CTL_STO)
#define PCF8584_MASK_CTL_STA  (1 << PCF8584_BIT_CTL_STA)
#define PCF8584_MASK_CTL_ENI  (1 << PCF8584_BIT_CTL_ENI)
#define PCF8584_MASK_CTL_ES   (3 << PCF8584_BIT_CTL_ES)
#define PCF8584_MASK_CTL_ES1  (1 << PCF8584_BIT_CTL_ES)
#define PCF8584_MASK_CTL_ES2  (2 << PCF8584_BIT_CTL_ES)
#define PCF8584_MASK_CTL_ESO  (1 << PCF8584_BIT_CTL_ESO)
#define PCF8584_MASK_CTL_PIN  (1 << PCF8584_BIT_CTL_PIN)

/* S2 register */
#define PCF8584_SCL_90K  (0 << 0)
#define PCF8584_SCL_45K  (1 << 0)
#define PCF8584_SCL_11K  (2 << 0)
#define PCF8584_SCL_1_5K (3 << 0)

#define PCF8584_CLK_3MHZ    (0 << 2)
#define PCF8584_CLK_4_43MHZ (4 << 2)
#define PCF8584_CLK_6MHZ    (5 << 2)
#define PCF8584_CLK_8MHZ    (6 << 2)
#define PCF8584_CLK_12MHZ   (7 << 2)

/* Function prototypes */
void print_usage(const char *program_name);
int hex_to_int(const char *hex_str);
void write_i2c(unsigned port, unsigned char device_addr, unsigned char *data, size_t length);
void read_i2c(unsigned port, unsigned char device_addr, size_t length);
void check_status(unsigned port);
void initialize_i2c(unsigned port);
static void wait_idle(unsigned port);
static void wait_tx(unsigned port);

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
    if (function != 'r' && function != 'w' && function != 's' && function != 'i') {
        fprintf(stderr, "Invalid function (must be r, w, i, or s)\n");
        return 1;
    }

    /* Parse I2C device address */
    unsigned char device_addr = (unsigned char)hex_to_int(argv[3]);
    if (device_addr == 0) {
        fprintf(stderr, "Invalid device address\n");
        return 1;
    }

    /* Handle each function */
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

            write_i2c(port, device_addr, buffer, data_length);
            break;
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

            read_i2c(port, device_addr, length);
            break;
        }

        case 's':
            check_status(port);
            break;

        case 'i':
            initialize_i2c(port);
            break;
    }

    return 0;
}

void print_usage(const char *program_name) {
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "  %s <port> w <device> <data>...  - Write data to I2C device\n", program_name);
    fprintf(stderr, "  %s <port> r <device> <length>   - Read data from I2C device\n", program_name);
    fprintf(stderr, "  %s <port> s <device>            - Check device status\n", program_name);
    fprintf(stderr, "  %s <port> i                     - Initialize the I2C controller at the given port\n", program_name);
    fprintf(stderr, "All numbers are hexadecimal\n");
}

int hex_to_int(const char *hex_str) {
    int result = 0;
    while (*hex_str) {
        char c = tolower(*hex_str++);
        if (c >= '0' && c <= '9')
            result = (result << 4) | (c - '0');
        else if (c >= 'a' && c <= 'f')
            result = (result << 4) | (c - 'a' + 10);
        else
            return 0;  /* Invalid hex character */
    }
    return result;
}

static void wait_idle(unsigned port) {
    /* TODO: need a max timeout for idle wait */
    unsigned status = inp(port + 1);
    // BB=0 -> bus is busy
    while ((status & PCF8584_MASK_STS_BB) == 0) {
        status = inp(port + 1);
    }
}

static void wait_tx(unsigned port) {
    unsigned status = inp(port + 1);
    // PIN=1 -> transmit not complete
    while ((status & PCF8584_MASK_STS_PIN) == PCF8584_MASK_STS_PIN) {
        status = inp(port + 1);
    }
}

void write_i2c(unsigned port, unsigned char device_addr, unsigned char *data, size_t length) {
    wait_idle(port);

    /* TODO: Implement actual I2C write protocol */
    /* For now, just write to the port */
    for (size_t i = 0; i < length; i++) {
        outp(port, data[i]);
    }
}

void read_i2c(unsigned port, unsigned char device_addr, size_t length) {
    /* TODO: Implement actual I2C read protocol */
    /* For now, just read from the port */
    for (size_t i = 0; i < length; i++) {
        putchar(inp(port));
    }
}

void check_status(unsigned port) {
    unsigned status = inp(port + 1);
    if ((status & PCF8584_MASK_STS_BB) == 0) {
        /* Busy */
        printf("Busy\n");
    } else {
        printf("Idle\n");
    }
}

void initialize_i2c(unsigned port) {
    /* Disable I2C bus, prepare to load self address to S0 */
    outp(port + 1, PCF8584_MASK_CTL_PIN);
    /* Set self address to 0xAA */
    outp(port, 0xAA >> 1);
    /* Prepare to load clock register (S2) */
    outp(port + 1, PCF8584_MASK_CTL_PIN | PCF8584_MASK_CTL_ES2);
    /* Set to ~4.43 MHz, 90 kHz SCL */
    outp(port, PCF8584_CLK_4_43MHZ | PCF8584_SCL_90K);
    /* Enable the I2C bus and go idle */
    outp(port + 1, PCF8584_MASK_CTL_PIN | PCF8584_MASK_CTL_ESO | PCF8584_MASK_CTL_ACK);
}
