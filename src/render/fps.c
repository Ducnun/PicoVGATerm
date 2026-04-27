#include "render/fps.h"

void draw_fps(int fps) {
    int x = 24;
    int y = 8;

    // Optional: draw "FPS:"
    renderer_draw_char(x, y, 'F', RED, WHITE); x += 8;
    renderer_draw_char(x, y, 'P', RED, WHITE); x += 8;
    renderer_draw_char(x, y, 'S', RED, WHITE); x += 8;
    renderer_draw_char(x, y, ':', RED, WHITE); x += 8;
    renderer_draw_char(x, y, ' ', RED, WHITE); x += 8;

    // Handle 0 explicitly
    if (fps == 0) {
        renderer_draw_char(x, y, '0', RED, WHITE);
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
        renderer_draw_char(x, y, digits[i], RED, WHITE);
        x += 8;
    }
}


void show_fps() {
    static int current_fps = 0;
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
    draw_fps(current_fps);
}
