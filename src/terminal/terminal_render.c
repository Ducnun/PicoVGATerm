#include "terminal/terminal_render.h"

void terminal_flush(void){
    const terminal_t* terminal = terminal_get_const();
    for (int y = 0; y < terminal->rows; y++) {

        if (!(terminal->lines[y].flags & LINE_FLAG_DIRTY))
            continue;

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