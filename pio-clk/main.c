/**
 * main.c
 *
 * Does nothing yet.
 */


#include "hardware/gpio.h"
#include "hardware/pio.h"
#include "pico/stdlib.h"


#include "clk6502.pio.h"


int main() {
  stdio_init_all();

  PIO  pio    = pio0;
  uint offset = pio_add_program(pio, &clk6502_program);
  uint sm     = pio_claim_unused_sm(pio, true);

  clk6502_program_init(pio, sm, offset);

  // Take the 6502 out of reset after at least two clock ticks
  // or 1 us.  Let's wait a little longer.
  sleep_us(5);
  gpio_put(PIN_RESB, 1);

  while (true) {
    const uint32_t request = pio_sm_get_blocking(pio, sm);

    // A 32-bit request is of the form:
    // ....... | A15-A0 | D7-D0 | R/W
    const int      do_read = (request & 0x0000001);
    const uint8_t  data    = (request & 0x00001FE) >> 1;
    const uint16_t address = (request & 0x1FFFE00) >> 9;

    if (do_read) {
      pio_sm_put_blocking(pio, sm, 0xEA);
    }
  }

  __builtin_unreachable();
}
