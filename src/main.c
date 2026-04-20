#include "hardware/pio.h"
#include "video/vga.h"
#include "render/fps.h"
#include "terminal/terminal.h"

#define VGA_PIO pio0

void fill_terminal(){
    char ch = 33;   // slowly cycle characters
    for (int y = 0; y < TERMINAL_ROWS; y++) {
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

    terminal_set_default_bg(VIOLET);
    terminal_set_default_fg(BLUE);
    set_font(&FONT_EVEREST);

    sleep_ms(10000);     // Time for the HDMI upscaler and grabber to wake up

    while (true) {
        show_fps();
        fill_terminal();
        for (int row = 0; row < TERMINAL_ROWS; row++) {
            terminal_scroll(1);
            sleep_ms(50);
        }
    }
}
