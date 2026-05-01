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

    uint8_t c = 33;


    while (true) {
        for (int y = 0; y < TERMINAL_ROWS + 4; y++){
           for (int x = 0; x < TERMINAL_COLS - 60; x++){
                terminal_print_char(c);
                // terminal_render_cursor();
                c++;
                if (c == 96) c = 33;
                sleep_ms(5);
            }
            terminal_newline();
        }
        for (int y = 0; y < TERMINAL_ROWS + 5; y++){
            for (int x = 0; x < TERMINAL_COLS - 60; x++){
                terminal_backspace();
                sleep_ms(10);
            }
        }
    }
}
