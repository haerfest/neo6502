/**
 * main.c
 *
 * Does nothing yet.
 */


#include <stdio.h>
#include <string.h>
#include "hardware/gpio.h"
#include "hardware/pio.h"
#include "pico/stdlib.h"
#include "clk6502.pio.h"


#define PIN_IRQB    25
#define PIN_RESB    26
#define PIN_NMIB    27


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


typedef struct {
  uint8_t reg[16];
  bool    is_irq_enabled_rtc;
  bool    is_irq_enabled_display;
} sheila_t;


sheila_t __uninitialized_ram(sheila);


static void sheila_init() {
  memset(&sheila, 0xFF, sizeof(sheila));

  sheila.is_irq_enabled_rtc     = true;
  sheila.is_irq_enabled_display = true;
  sheila.reg[0x00] = 0x02;
}


static inline void  __time_critical_func(sheila_write)(uint8_t reg, uint8_t value) {
  printf("SHEILA w &FEX%X <- &%02X\n", reg, value);

  switch (reg) {
    case 0x00:
      sheila.is_irq_enabled_display = (value & 0x04) != 0;
      sheila.is_irq_enabled_rtc     = (value & 0x08) != 0;
      break;

    case 0x05:
      if (value & 0x01) sheila.reg[0x00] &= ~0x04;
      if (value & 0x02) sheila.reg[0x00] &= ~0x08;

      if ((sheila.reg[0x00] & 0x7C) == 0) {
        sheila.reg[0x00] &= ~0x01;
        gpio_put(PIN_IRQB, 1);
      }
      break;

    default:
      sheila.reg[reg] = value;
      break;
  }
}


static inline uint8_t __time_critical_func(sheila_read)(uint8_t reg) {
  printf("SHEILA r &FEX%X\n", reg);

  const uint8_t value = sheila.reg[reg];
  
  if (reg == 0x00)
    sheila.reg[0x00] &= ~0x02;
    
  return value;
}


// The 64K memory address space is stored in RAM and left uninitialized.
uint8_t __uninitialized_ram(memory)[65536];


int main() {
  stdio_init_all();

  // For debugging purposes.
  uart_init(uart0, 115200);
  gpio_set_function(28, GPIO_FUNC_UART);
  gpio_set_function(29, GPIO_FUNC_UART);
  printf("Build " __DATE__ " " __TIME__ "\n");

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

  // Load in the OS. Skip BASIC for now.
  memcpy(&memory[0xC000], ROM_OS, 0x4000);

  PIO  pio    = pio0;
  uint offset = pio_add_program(pio, &clk6502_program);
  uint sm     = pio_claim_unused_sm(pio, true);

  clk6502_program_init(pio, sm, offset);

  // Take the 6502 out of reset after at least two cycles (1 ns), so we
  // wait two.
  sleep_us(2);
  gpio_put(PIN_RESB, 1);

  uint64_t cycles     = 0;
  bool     do_rtc_irq = true;

  while (true) {
    const uint32_t request   = pio_sm_get_blocking(pio, sm);
    const uint16_t address   = request >> 16;
    const bool     is_sheila = (address >> 8) == PAGE_SHEILA;

    printf("%08X\n", request);

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

    // Every clock tick the 6502 will perform a read or a write, so we can
    // use this to count cycles.
    cycles++;

#if false
    // There are two periodic interrupts that together make up a 100 Hz timer:
    // 1. A 50 Hz display end interrupt.
    // 2. A 50 Hz clock interrupt.
    // At 100 Hz combined we need to alternatingly generate one of these every
    // 2,000,000 / 100 = 20,000 cycles.
    if ((cycles % 20000) == 0) {
      if (do_rtc_irq)
        sheila.reg[0x00] |= sheila.is_irq_enabled_rtc     ? 0x09 : 0x00;
      else
        sheila.reg[0x00] |= sheila.is_irq_enabled_display ? 0x05 : 0x00;

      if (sheila.is_irq_enabled_rtc || sheila.is_irq_enabled_display)
        gpio_put(PIN_IRQB, 0);

      do_rtc_irq = !do_rtc_irq;
    }
#endif
  }

  __builtin_unreachable();
}
