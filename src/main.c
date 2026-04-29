#include "hardware/pio.h"
#include "video/vga.h"
#include "render/fps.h"
#include "terminal/terminal.h"

#define VGA_PIO pio0

int main() {
    stdio_init_all();

    // Setup VGA video signal generation
    start_vga(VGA_PIO);

    terminal_set_default_bg(BLACK);
    terminal_set_default_fg(GREEN);
    set_font(&FONT_ENVIOUS);

    // sleep_ms(10000);     // Time for the HDMI upscaler and grabber to wake up

    while (true) {
        uint8_t c = 33;
        for (int y = 0; y < TERMINAL_ROWS; y++){
            for (int x = 0; x < TERMINAL_COLS - 40; x++){
                terminal_print_char(c);
                c++;
                if (c == 96) c = 33;
                sleep_ms(1);
            }
            terminal_newline();
        }
    }
}
