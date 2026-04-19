#include "hardware/pio.h"
#include "video/vga.h"
#include "render/renderer.h"

#define VGA_PIO pio0
#define LEFT_PAD 16

void draw_fps(framebuffer_t fb, int fps) {
    int x = 24;
    int y = 8;

    // Optional: draw "FPS:"
    draw_char(fb, x, y, 'F', RED, WHITE); x += 8;
    draw_char(fb, x, y, 'P', RED, WHITE); x += 8;
    draw_char(fb, x, y, 'S', RED, WHITE); x += 8;
    draw_char(fb, x, y, ':', RED, WHITE); x += 8;
    draw_char(fb, x, y, ' ', RED, WHITE); x += 8;

    // Handle 0 explicitly
    if (fps == 0) {
        draw_char(fb, x, y, '0', RED, WHITE);
        return;
    }

    // Extract digits (reverse order)
    char digits[10];
    int count = 0;

    while (fps > 0 && count < 10) {
        digits[count++] = '0' + (fps % 10);
        fps /= 10;
    }

    // Draw in correct order
    for (int i = count - 1; i >= 0; i--) {
        draw_char(fb, x, y, digits[i], RED, WHITE);
        x += 8;
    }
}

int current_fps = 0;

void fps_update(framebuffer_t fb) {
    static uint32_t frame_count = 0;
    static absolute_time_t last_time;
    static bool initialized = false;

    if (!initialized) {
        last_time = get_absolute_time();
        initialized = true;
    }

    frame_count++;

    absolute_time_t now = get_absolute_time();
    int64_t elapsed = absolute_time_diff_us(last_time, now);

    if (elapsed >= 1000000) {
        current_fps = frame_count;
        frame_count = 0;
        last_time = now;
    }

    // Draw every frame (cheap enough)
    draw_fps(fb, current_fps);
}

void fill_terminal(){
    char ch = 33;   // slowly cycle characters
    for (int y = 0; y < TERMINAL_ROWS; y++) {
        for (int x = 0; x < TERMINAL_COLS; x++) {

            set_char(x, y, ch);

            ch++;
            if (ch >= 33 + 96) ch = 33;
        }
    }
}

int main() {
    stdio_init_all();

    // Setup VGA video signal generation
    start_vga(VGA_PIO);

    set_default_bg(BLUE);
    set_default_fg(VIOLET);
    fill_terminal();

    // Just draws garbage to all cells using draw char with iterated values
    while (true) {
        // Now inserting in the screenbuffer instead

        framebuffer_t fb = get_framebuffer();
        render_terminal(fb);
        // fps_update(fb); // Track FPS ON SCREEN!!
        // sleep_ms(1000);
    }
}
