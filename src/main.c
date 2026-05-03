#include <stdio.h>
#include "pico/stdlib.h"

#include "video/vga.h"
#include "render/fps.h"
#include "terminal/terminal.h"
#include "stream/stream.h"

#define VGA_PIO pio0

int main() {
    stdio_init_all();

    // Setup VGA video signal generation
    start_vga(VGA_PIO);

    // Setup UART inputs
    stream_uart_init();

    // Setup terminal settings
    terminal_set_default_bg(BLUE);
    terminal_set_default_fg(VIOLET);
    set_font(&FONT_ENVIOUS);

    // Wait for USB to be connected
    while (!stdio_usb_connected()) {
        sleep_ms(10);
    }
    printf("Connected!\n");
    printf("Running PicoVGATerm\n");

    while (true) {
        stream_poll_event();
        tight_loop_contents();  
    }
}
