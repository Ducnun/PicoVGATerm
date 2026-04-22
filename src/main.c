#include "hardware/pio.h"
#include "video/vga.h"
#include "render/fps.h"
#include "terminal/terminal.h"

#define VGA_PIO pio0

void fill_terminal(){
    char ch = 33;   // slowly cycle characters
    for (int y = TERMINAL_ROWS - 1; y < TERMINAL_ROWS; y++) {
        for (int x = 0; x < TERMINAL_COLS; x++) {

            terminal_set_char(x, y, ch);

            if (ch == 33 + 96) ch = 33;
            else{
                ch++;
            }
        }
    }
}

int main() {
    stdio_init_all();

    // Setup VGA video signal generation
    start_vga(VGA_PIO);

    terminal_set_default_bg(BLACK);
    terminal_set_default_fg(GREEN);
    set_font(&FONT_ENVIOUS);

    // sleep_ms(10000);     // Time for the HDMI upscaler and grabber to wake up

    while (true) {
        for (int row = 0; row < TERMINAL_ROWS / 1; row++) {
            fill_terminal();
            terminal_scroll(1);
            sleep_ms(250);
        }
        for (int row = 0; row < TERMINAL_ROWS /1; row++){
            terminal_scroll(1);
            sleep_ms(250);
        }
        // show_fps();
    }
}
