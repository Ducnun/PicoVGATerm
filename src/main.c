#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "vga.h"

#define PICO_DEFAULT_LED_PIN 25

int main() {

    stdio_init_all();

    const uint LED_PIN = PICO_DEFAULT_LED_PIN;

    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    PIO pio = pio0;
    uint sm = pio_claim_unused_sm(pio, true);
    uint offset = pio_add_program(pio, &hsync_program);

    start_hsync(pio, sm, offset);
    start_vsync_hook();

    while (true) {
        gpio_put(LED_PIN, 1);
        sleep_ms(500);

        gpio_put(LED_PIN, 0);
        sleep_ms(500);
    }
}