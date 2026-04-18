#include "hardware/pio.h"
#include "video/vga.h"

#define VGA_PIO pio0

int main() {
    stdio_init_all();

    // Setup VGA video signal generation
    start_vga(VGA_PIO);

    while (true) {
        tight_loop_contents();
    }
}
