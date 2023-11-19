/**
 * main.c
 *
 * Uses PIO (Programmable I/O) to drive the 65C02 at 2 MHz, reading and writing
 * bytes via the FIFOs.
 */


#include "hardware/gpio.h"
#include "hardware/pio.h"
#include "pico/stdlib.h"
#include "clk6502.pio.h"


#define PIN_IRQB     25
#define PIN_RESB     26
#define PIN_NMIB     27

#define OPCODE_NOP   0xEA

#define MAX_REQUESTS 50


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
  uint32_t requests[MAX_REQUESTS];
  uint     request_index = 0;
 
  stdio_init_all();

	// Initialise the NMIB, RESB and IRQB pins.
  gpio_init(PIN_NMIB);
  gpio_init(PIN_RESB);
  gpio_init(PIN_IRQB);

  // The NMIB, IRQB and RESB pins are always outputs.
  gpio_set_dir(PIN_NMIB, GPIO_OUT);
  gpio_set_dir(PIN_IRQB, GPIO_OUT);
  gpio_set_dir(PIN_RESB, GPIO_OUT);

  // Do not assert /IRQ nor /NMI.
  gpio_put(PIN_NMIB, 1);
  gpio_put(PIN_IRQB, 1);

  // Assert /RESET.
  gpio_put(PIN_RESB, 0);

  PIO  pio    = pio0;
  uint offset = pio_add_program(pio, &clk6502_program);
  uint sm     = pio_claim_unused_sm(pio, true);

  clk6502_program_init(pio, sm, offset);

	// Take the 6502 out of reset after at least two cycles (1 ns), so we
  // wait two.
  sleep_us(2);
  gpio_put(PIN_RESB, 1);

  while (true) {
    const uint32_t request = pio_sm_get_blocking(pio, sm);

    // Store the first N requests for debugging.
    if (request_index < MAX_REQUESTS)
      requests[request_index++] = request;

    // Upon a read request, return a NOP.
    if (request & 1)
      pio_sm_put_blocking(pio, sm, OPCODE_NOP);
  }

  __builtin_unreachable();
}
