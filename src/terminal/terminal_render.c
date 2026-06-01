#include "terminal/terminal_render.h"
#include "pico/stdlib.h"
#include "stdio.h"

render_line_cache_t line_cache[80];

void terminal_flush(void){
    const terminal_t* terminal = terminal_get_const();
    for (int y = 0; y < terminal->rows; y++) {
        terminal_line_t* line = &(terminal->lines[y]);

        if (line->flags & LINE_FLAG_FULL_DIRTY){
            uint16_t range_to_render = MAX(line_cache->rendered_length, line->length);
            printf("rtr: %d, length: %d, rendered_length: %d\n", range_to_render, line->length, line_cache->rendered_length);

            for (int x = 0; x < range_to_render; x++) {
                const terminal_cell_t* cell = terminal_get_cell_const(x, y);
                terminal_clear_cell_flags(x, y, CELL_FLAG_DIRTY);
                renderer_draw_char(x * FONT->width,y * FONT->height,cell->c,cell->fg,cell->bg);
            }
            if (line->length < line_cache->rendered_length) {
                renderer_fill_rectangle(
                    line->length * FONT->width,
                    y * FONT->height,
                    FONT->height,
                    (terminal->cols - line->length) * FONT->width,
                    terminal->default_bg
                );
            }
            line_cache->rendered_length = line->length;
        }

        else if (line->flags & LINE_FLAG_PARTIAL_DIRTY){
            // Take cursor into account
            uint16_t range_to_render = MAX(line_cache->rendered_length, line->length);
            printf("rtr: %d, length: %d, rendered_length: %d\n", range_to_render, line->length, line_cache->rendered_length);

            for (int x = 0; x < range_to_render; x++) {
                const terminal_cell_t* cell = terminal_get_cell_const(x, y);
                if (cell->flags & CELL_FLAG_DIRTY){
                    renderer_draw_char(x * FONT->width,y * FONT->height,cell->c,cell->fg,cell->bg);
                    terminal_clear_cell_flags(x, y, CELL_FLAG_DIRTY);
                }
            }
            if (line->length < line_cache->rendered_length) {
                renderer_fill_rectangle(
                    line->length * FONT->width,
                    y * FONT->height,
                    FONT->height,
                    (terminal->cols - line->length) * FONT->width,
                    terminal->default_bg
                );
            }
            line_cache->rendered_length = line->length;
        }

        line->flags &= ~(LINE_FLAG_FULL_DIRTY | LINE_FLAG_PARTIAL_DIRTY);
    }
}

void terminal_render_cursor(void){
    static uint32_t last_time;
    static bool flash;

    const terminal_t* terminal = terminal_get_const();
    if (!terminal->cursor->visible) return;

    int px = terminal->cursor->x * FONT->width;
    int py = terminal->cursor->y * FONT->height;

    // simple version: invert block

    if (to_ms_since_boot(get_absolute_time()) > last_time + terminal->cursor->delay){
        if (flash){
            renderer_fill_rectangle(px, py, FONT->height, FONT->width, terminal->default_fg);
        }
        else {
            renderer_fill_rectangle(px, py, FONT->height, FONT->width, terminal->default_bg);
        }
        flash = !flash;
        last_time = to_ms_since_boot(get_absolute_time());
    }
}
