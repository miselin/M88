/*
 * SBC8088_SMC.c
 *
 * Created: 2/05/2024 7:50:53 PM
 * Author : Matthew Iselin
 */

#define F_CPU 9600000  // 9.6 MHz

#include <avr/delay.h>
#include <avr/io.h>

// Main signals:
// - PB0 = PWR_GOOD = ATX POWER OK = +5V when power has stabilized
// - PB1 = PWR_SWITCH = switch becomes +5V when pushed (momentary)
// - PB2 = RESET# = emitted reset signal, active low
// - PB3 = RESET = emitted reset signal, active high
// - PB4 = PS_ON# = when low, ATX power supply will push power
// - PB5 = reset

enum State {
  // SMC is waiting for a power switch toggle
  AWAITING_POWER_SWITCH,
  // SMC is waiting for POWER OK
  AWAITING_POWER_OK,
  // SMC is holding reset but power is good
  HOLDING_RESET,
  // SMC is complete
  SYSTEM_READY,
};

static enum State state = AWAITING_POWER_SWITCH;

int main(void) {
  state = AWAITING_POWER_SWITCH;

  // set output/input directions
  DDRB = (1 << PORTB2) | (1 << PORTB3) | (1 << PORTB4);
  DDRB &= ~((1 << PORTB0) | (1 << PORTB1) | (1 << PORTB5));

  // set default pin states - PS_ON=1, RESET=0, RESET#=1
  PORTB |= (1 << PORTB2) | (1 << PORTB4);
  PORTB &= ~(1 << PORTB3);

  while (1) {
    switch (state) {
      case AWAITING_POWER_SWITCH:
        if (PINB & (1 << PINB1)) {
          // slight debounce
          _delay_ms(50);

          if (PINB & (1 << PINB1)) {
            // emit PS_ON#
            PORTB &= ~(1 << PORTB4);

            state = AWAITING_POWER_OK;
          }
        }
        break;
      case AWAITING_POWER_OK:
        if (PINB & (1 << PINB0)) {
          // set active-high reset
          PORTB |= 1 << PORTB3;
          // set active-low reset
          PORTB &= ~(1 << PORTB2);

          state = HOLDING_RESET;
        }
        break;
      case HOLDING_RESET:
        // hold reset for 50ms
        _delay_ms(50);

        // release the active-high reset pin
        PORTB &= ~(1 << PORTB3);

        // hold a little longer
        _delay_ms(10);

        // now release the active-low reset pin
        PORTB |= 1 << PORTB2;

        state = SYSTEM_READY;
        break;
      case SYSTEM_READY:
        // TODO: check for power switch held down to power off the machine?
        break;
    }

    _delay_ms(50);
  }
}
