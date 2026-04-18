#include "hardware/pio.h"
#include "video/vga.h"
#include "render/renderer.h"

#define VGA_PIO pio0
#define LEFT_PAD 16

int main() {
    stdio_init_all();

    // Setup VGA video signal generation
    start_vga(VGA_PIO);

    framebuffer_t fb = get_framebuffer();
    draw_char(fb, LEFT_PAD + 0, 0, 'H', WHITE, BLACK);
    draw_char(fb, LEFT_PAD + 8, 0, 'e', WHITE, BLACK);
    draw_char(fb, LEFT_PAD + 16, 0, 'l', WHITE, BLACK);
    draw_char(fb, LEFT_PAD + 24, 0, 'l', WHITE, BLACK);
    draw_char(fb, LEFT_PAD + 32, 0, 'o', WHITE, BLACK);
    draw_char(fb, LEFT_PAD + 48, 0, 'W', WHITE, BLACK);
    draw_char(fb, LEFT_PAD + 56, 0, 'o', WHITE, BLACK);
    draw_char(fb, LEFT_PAD + 64, 0, 'r', WHITE, BLACK);
    draw_char(fb, LEFT_PAD + 72, 0, 'l', WHITE, BLACK);
    draw_char(fb, LEFT_PAD + 80, 0, 'd', WHITE, BLACK);
    draw_char(fb, LEFT_PAD + 88, 0, '!', WHITE, BLACK);

    while (true) {
        tight_loop_contents();
    }
}
