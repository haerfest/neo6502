/**
 * main.c
 *
 * Does nothing yet.
 */


#include "hardware/gpio.h"
#include "hardware/pio.h"
#include "pico/stdlib.h"
#include "clk6502.pio.h"


#define PIN_AUDIO 20


int main() {
  stdio_init_all();

  sleep_ms(5000);

  gpio_init(PIN_AUDIO);
  gpio_set_dir(PIN_AUDIO, GPIO_OUT);
  gpio_put(PIN_AUDIO, 0);

  for (int i = 0; i < 50; i++) {
    gpio_put(PIN_AUDIO, 1);
    sleep_ms(5);
    gpio_put(PIN_AUDIO, 0);
    sleep_ms(5);
  }

  PIO  pio    = pio0;
  uint offset = pio_add_program(pio, &clk6502_program);
  uint sm     = pio_claim_unused_sm(pio, true);

  clk6502_program_init(pio, sm, offset);

  // Take the 6502 out of reset.
  gpio_put(PIN_RESB, 1);

  while (true) {
    const uint32_t request = pio_sm_get_blocking(pio, sm);

    // A 32-bit request is of the form:
    // ....... | A15-A0 | D7-D0 | R/W
    const int      do_read = (request & 0x0000001);
    const uint8_t  data    = (request & 0x00001FE) >> 1;
    const uint16_t address = (request & 0x1FFFE00) >> 9;

    if (do_read) {
      // RESET vector points to 0x1234, otherwise return NOP (0xEA).
      const uint8_t response =
        address == 0xFFFC ? 0x34 :
        address == 0xFFFD ? 0x12 : 0xEA;
      pio_sm_put_blocking(pio, sm, response);
    }
  }

  __builtin_unreachable();
}
