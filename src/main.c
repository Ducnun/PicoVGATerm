#include "hardware/pio.h"
#include "video/vga.h"
#include "render/fps.h"
#include "terminal/terminal.h"
#include <stdio.h>
#include "pico/stdlib.h"

#define VGA_PIO pio0

int main() {
    stdio_init_all();

    // Setup VGA video signal generation
    start_vga(VGA_PIO);

    terminal_set_default_bg(BLUE);
    terminal_set_default_fg(VIOLET);
    set_font(&FONT_ENVIOUS);

    // sleep_ms(10000);     // Time for the HDMI upscaler and grabber to wake up

    while (!stdio_usb_connected()) {
        sleep_ms(10);
    }
    printf("Connected!\n");
    printf("Running PicoVGATerm\n");

    uint8_t c = '0';


    while (true) {
        for (int y = 0; y < TERMINAL_ROWS - 1; y++){
           for (int x = 0; x < TERMINAL_COLS; x++){
                terminal_print_char(c);
                // terminal_render_cursor();
                c++;
                if (c == 96) c = 33;
                sleep_ms(2);
            }
            // terminal_newline();
        }
        for (int y = 0; y < TERMINAL_ROWS - 58; y++){
           for (int x = 0; x < TERMINAL_COLS; x++){
                terminal_print_char(c);
                terminal_render_cursor();
                c++;
                if (c == 96) c = 33;
                sleep_ms(500);
            }
            // terminal_newline();
        }
        for (int y = 0; y < TERMINAL_ROWS - 56; y++){
            for (int x = 0; x < TERMINAL_COLS; x++){
                terminal_backspace();
                sleep_ms(100);
            }
        }
    }
}
