/**
 * main.c
 *
 * Does nothing yet.
 */


#include "hardware/gpio.h"
#include "hardware/pio.h"
#include "pico/stdlib.h"
#include "clk6502.pio.h"


#define PIN_AUDIO    20

#define OPCODE_NOP   0xEA

#define MAX_REQUESTS 50


int main() {
  uint32_t requests[MAX_REQUESTS];
  uint     request_index = 0;

  stdio_init_all();

  sleep_ms(2000);

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

    if (request_index < MAX_REQUESTS) {
      requests[request_index++] = request;
    }

    if (request & 1) {
      pio_sm_put_blocking(pio, sm, OPCODE_NOP);
    }
  }

  __builtin_unreachable();
}
