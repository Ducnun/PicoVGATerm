#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "vga.h"

int main() {

    stdio_init_all();

    const uint LED_PIN = PICO_DEFAULT_LED_PIN;

    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    // Select timing table and setup hsync program
    timing = &vga_640x480;
    PIO pio = pio0;
    uint sm = pio_claim_unused_sm(pio, true);
    uint offset = pio_add_program(pio, &hsync_program);

    // Set all RGB pins high to mitigate black screen
    for (uint8_t i = R_MSB_PIN; i <= B_LSB_PIN; i++) {
        gpio_init(i);
        gpio_set_dir(i, GPIO_OUT);
        gpio_put(i, 1);
    }

    // Preempt vsync hook
    start_vsync_hook();
    start_hsync(pio, sm, offset);

    // Setup scanline state machine
    sm = pio_claim_unused_sm(pio, true);
    offset = pio_add_program(pio, &scanline_out_program);
    start_scanline_out(pio, sm, offset);

    while (true) {
        render_scanline();
    }
}