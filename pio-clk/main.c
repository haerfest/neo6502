/**
 * main.c
 *
 * Does nothing yet.
 */


#include "hardware/clocks.h"
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

  uint request;
  while (true) {
    request = pio_sm_get_blocking(pio, sm);
    if (request & 1 == 0) {
      // Write request, ignore.
    } else {
      // Read request, return NOP.
      pio_sm_put_blocking(pio, sm, 0xEA);
    }
  }

  __builtin_unreachable();
}
