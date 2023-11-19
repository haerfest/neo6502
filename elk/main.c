/**
 * main.c
 *
 * Does nothing yet.
 */


#include <string.h>
#include "hardware/gpio.h"
#include "hardware/pio.h"
#include "pico/stdlib.h"
#include "clk6502.pio.h"


// The SHEILA hardware registers live at 0xFExx.
#define PAGE_SHEILA 0xFE

                                   
// The Electron OS 1.00 ROM.
const uint8_t ROM_OS[] = {
#include "os.hex"
};

// The BBC BASIC 2 ROM.
const uint8_t ROM_BASIC[] = {
#include "basic.hex"
};


// The 64K memory address space is stored in RAM and left uninitialized.
uint8_t __uninitialized_ram(memory)[65536];


static inline void  __time_critical_func(sheila_write)(uint8_t reg, uint8_t byte) {
}


static inline uint8_t __time_critical_func(sheila_read)(uint8_t reg) {
  return 0xFF;
}


int main() {
  stdio_init_all();

  // Load in the OS. Skip BASIC for now.
  memcpy(&memory[0xC000], ROM_OS, 0x4000);

  PIO  pio    = pio0;
  uint offset = pio_add_program(pio, &clk6502_program);
  uint sm     = pio_claim_unused_sm(pio, true);

  clk6502_program_init(pio, sm, offset);

  // Take the 6502 out of reset.
  gpio_put(PIN_RESB, 1);

  while (true) {
    const uint32_t request   = pio_sm_get_blocking(pio, sm);
    const uint16_t address   = request >> 16;
    const bool     is_sheila = (address >> 8) == PAGE_SHEILA;

    if (request & 1) {
      // Read request.
      const uint8_t byte = is_sheila ? sheila_read(address & 0x000F) : memory[address];
      pio_sm_put_blocking(pio, sm, byte);
    } else {
      // Write request.
      const uint8_t byte = request >> 8;
      if (is_sheila)
        sheila_write(address & 0x000F, byte);
      else if (address < 0x8000)
        memory[address] = byte;
    }        
  }

  __builtin_unreachable();
}
