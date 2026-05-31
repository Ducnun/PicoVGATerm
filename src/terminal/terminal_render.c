#include "terminal/terminal_render.h"

void terminal_flush(void){
    const terminal_t* terminal = terminal_get_const();
    for (int y = 0; y < terminal->rows; y++) {

        if (!(terminal->lines[y].flags & LINE_FLAG_DIRTY))
            continue;

        if (terminal->lines[y].flags & LINE_FLAG_CLEARED){
            renderer_fill_rectangle(0, y, FONT->height, terminal->cols * FONT->width, terminal->default_bg);
            terminal->lines[y].flags &= ~LINE_FLAG_CLEARED;
            continue;
        }

        for (int x = 0; x < terminal->cols; x++) {
            const terminal_cell_t* cell = terminal_get_cell_const(x, y);

            renderer_draw_char(
                x * FONT->width,
                y * FONT->height,
                cell->c,
                cell->fg,
                cell->bg
            );
        }

        terminal->lines[y].flags &= ~LINE_FLAG_DIRTY;
    }
}

void terminal_render_cursor(void){
    const terminal_t* terminal = terminal_get_const();
    if (!terminal->cursor->visible) return;

    int px = terminal->cursor->x * FONT->width;
    int py = terminal->cursor->y * FONT->height;

    // simple version: invert block
    renderer_fill_rectangle(px, py + FONT->height - 2, 2, FONT->width, terminal->default_fg);
}
