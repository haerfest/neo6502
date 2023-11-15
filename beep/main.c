/**
 * main.c
 *
 * Beeps the speaker intermittently.
 */


#include "hardware/gpio.h"
#include "pico/stdlib.h"


// On the Neo6502 the audio path is connected to GPIO20.
const uint SOUND_PIN = 20;

int main() {
  // Set SOUND_PIN to output.
  gpio_init(SOUND_PIN);
  gpio_set_dir(SOUND_PIN, GPIO_OUT);

  while (true) {
    // This generates a square wave with a period of 10 ms, which gives
    // a tone frequency of 100 Hz. The duration is 50 cycles, or half
    // a second.
    for (int i = 0; i < 50; i++) {
      gpio_put(SOUND_PIN, 1);
      sleep_ms(5);

      gpio_put(SOUND_PIN, 0);
      sleep_ms(5);
    }

    // Then stay silent for two seconds.
    sleep_ms(2000);
  }

  __builtin_unreachable();
}
