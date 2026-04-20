#include "hardware/pio.h"
#include "video/vga.h"
#include "render/fps.h"
#include "terminal/terminal.h"

#define VGA_PIO pio0

void fill_terminal(){
    char ch = '0';   // slowly cycle characters
    for (int y = 0; y < TERMINAL_ROWS; y++) {
        for (int x = 0; x < TERMINAL_COLS; x++) {

            set_char(x, y, ch);

            if (ch == '9') ch = '0';
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

    set_default_bg(VIOLET);
    set_default_fg(BLUE);
    set_font(&FONT_EVEREST);
    fill_terminal();

    while (true) {
        show_fps();
    }
}
