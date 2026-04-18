#include "hardware/pio.h"
#include "video/vga.h"
#include "render/renderer.h"

#define VGA_PIO pio0
#define LEFT_PAD 16

void draw_fps(framebuffer_t fb, int fps) {
    int x = 24;
    int y = 8;

    // Optional: draw "FPS:"
    draw_char(fb, x, y, 'F', WHITE, BLACK); x += 8;
    draw_char(fb, x, y, 'P', WHITE, BLACK); x += 8;
    draw_char(fb, x, y, 'S', WHITE, BLACK); x += 8;
    draw_char(fb, x, y, ':', WHITE, BLACK); x += 8;
    draw_char(fb, x, y, ' ', WHITE, BLACK); x += 8;

    // Handle 0 explicitly
    if (fps == 0) {
        draw_char(fb, x, y, '0', WHITE, BLACK);
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
        draw_char(fb, x, y, digits[i], WHITE, BLACK);
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

int main() {
    stdio_init_all();

    // Setup VGA video signal generation
    start_vga(VGA_PIO);

    framebuffer_t fb = get_framebuffer();

    uint8_t t = 0;

    // Just draws garbage to all cells using draw char with iterated values
    while (true) {
        char ch = 33 + (t & 31);   // slowly cycle characters
        color_t color = t & 7;
        color_t bg_color = (t >> 3) & 7;

        for (int y = 30; y < fb.height; y += 9) {
            for (int x = 10; x < fb.width; x += 9) {

                draw_char(fb, x, y, ch, color, bg_color);

                ch++;
                if (ch >= 33 + 96) ch = 33;

                color++;
                bg_color++;

                if (color >= 8) color = 0;
                if (bg_color >= 8) bg_color = 0;
            }
        }
        fps_update(fb); // Track FPS ON SCREEN!!

        t++;  // drives animation
    }
}
