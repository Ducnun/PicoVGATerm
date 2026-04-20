#include "hardware/pio.h"
#include "video/vga.h"
#include "render/renderer.h"
#include "render/fps.h"

#define VGA_PIO pio0
#define LEFT_PAD 16

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
    framebuffer_t fb = get_framebuffer();

    set_default_bg(VIOLET);
    set_default_fg(BLUE);
    fill_terminal();

    while (true) {
        render_terminal(fb);
        show_fps(fb);
    }
}
