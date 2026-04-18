#include "hardware/pio.h"
#include "video/vga.h"
#include "render/renderer.h"

#define VGA_PIO pio0

int main() {
    stdio_init_all();

    // Setup VGA video signal generation
    start_vga(VGA_PIO);

    framebuffer_t fb = get_framebuffer();
    draw_char(fb, 0, 0, 'H', WHITE, BLACK);
    draw_char(fb, 8, 0, 'E', WHITE, BLACK);
    draw_char(fb, 16, 0, 'L', WHITE, BLACK);
    draw_char(fb, 24, 0, 'L', WHITE, BLACK);
    draw_char(fb, 32, 0, 'O', WHITE, BLACK);
    draw_char(fb, 48, 0, 'W', WHITE, BLACK);
    draw_char(fb, 56, 0, 'O', WHITE, BLACK);
    draw_char(fb, 64, 0, 'R', WHITE, BLACK);
    draw_char(fb, 72, 0, 'L', WHITE, BLACK);
    draw_char(fb, 80, 0, 'D', WHITE, BLACK);
    draw_char(fb, 88, 0, '!', WHITE, BLACK);

    while (true) {
        tight_loop_contents();
    }
}
