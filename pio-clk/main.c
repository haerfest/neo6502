/**
 * main.c
 *
 * Does nothing yet.
 */


#include "hardware/gpio.h"
#include "hardware/pio.h"
#include "pico/stdlib.h"
#include "clk6502.pio.h"


#define OPCODE_NOP 0xEA


/*
  The first N requests can look like this (there is some variability when resetting):

  $30fb 00 01  (In reset).
  $30fb 00 01  (In reset).
  $ffff 00 01  Seems to indicate 65C02 is coming out of /RESET.
  $30fb 00 01  \
  $01f8 00 01   \ Part of reset
  $01f7 00 01   /   procedure
  $01f6 00 01  /
  $fffc 00 01  Read low  byte of RST vector (NOP, so $EA).
  $fffd 00 01  Read high byte of RST vector (NOP, so PC is now $EAEA).
  $eaea 00 01  Start reading from PC.
  $eaeb 00 01  Read next instruction from $EAEB.
  $eaeb 00 01  Again, since NOP takes two cycles.
  $eaec 00 01  Read next instruction from $EAEC.
  $eaec 00 01  Again.
  $eaed 00 01  Read next instruction from $EAED.
  $eaed 00 01  Again, and so on.
  :
*/
  
int main() {
  stdio_init_all();

  PIO  pio    = pio0;
  uint offset = pio_add_program(pio, &clk6502_program);
  uint sm     = pio_claim_unused_sm(pio, true);

  clk6502_program_init(pio, sm, offset);

  // Take the 6502 out of reset.
  gpio_put(PIN_RESB, 1);

  while (true) {
    const uint32_t request = pio_sm_get_blocking(pio, sm);

    // Upon a read request, return a NOP.
    if (request & 1) {
      pio_sm_put_blocking(pio, sm, OPCODE_NOP);
    }
  }

  __builtin_unreachable();
}
