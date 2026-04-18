#include "hardware/pio.h"
#include "video/vga.h"

#define VGA_PIO pio0

int main() {

    stdio_init_all();

    const uint LED_PIN = PICO_DEFAULT_LED_PIN;

    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    // Setup scanline state machine
    start_vga(VGA_PIO);

    // int pin = 2;
    // gpio_init(pin);
    // gpio_set_dir(pin, GPIO_OUT);
    // gpio_put(pin, 1);

    gpio_put(LED_PIN, true);

    render_hello_world();
    
    while (true) {
        tight_loop_contents();
        // render_animation();
        // sleep_ms(100);s
    }
}
