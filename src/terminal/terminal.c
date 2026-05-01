#include "terminal/terminal.h"

static terminal_cell_t screencells[TERMINAL_COLS * TERMINAL_ROWS];
static uint8_t viewport_start = 0;
static uint8_t terminal_default_fg = color_lut[WHITE];
static uint8_t terminal_default_bg = color_lut[BLACK];

static inline terminal_cell_t* terminal_get_cell(uint16_t px, uint16_t py){
    return screencells + ((viewport_start + py) % TERMINAL_ROWS * TERMINAL_COLS + px);
}

void terminal_set_default_fg(color_t color){
    terminal_default_fg = color_lut[color];
}

void terminal_set_default_bg(color_t color){
    terminal_default_bg = color_lut[color];
}

void terminal_print_char(char c){
    terminal_set_char(terminal_cursor.x, terminal_cursor.y, c);

    uint16_t new_x = terminal_cursor.x + 1;
    uint16_t new_y = terminal_cursor.y;

    // Horizontal wrap
    if (new_x >= TERMINAL_COLS){
        new_x = 0;
        new_y++;
    }
    // Vertical wrap
    if (new_y >= TERMINAL_ROWS){
        terminal_scroll(1);
        new_y = TERMINAL_ROWS - 1;
    }
    terminal_move_cursor(new_x, new_y);
}

void terminal_backspace(void){
    // Nothing to do at absolute top-left
    if (terminal_cursor.x == 0 && terminal_cursor.y == 0){
        return;
    }

    uint16_t new_x = terminal_cursor.x;
    uint16_t new_y = terminal_cursor.y;

    if (new_x > 0){
        new_x--;
    } else {
        // move to end of previous line
        new_y--;
        new_x = TERMINAL_COLS - 1;
    }

    // Move cursor FIRST (this also clears old cursor visually)
    terminal_move_cursor(new_x, new_y);

    // Then erase character at new position
    terminal_rm_char(new_x, new_y);
}

void terminal_newline(void){
    uint16_t new_x = 0;
    uint16_t new_y = terminal_cursor.y + 1;

    if (new_y >= TERMINAL_ROWS){
        terminal_scroll(1);
        new_y = TERMINAL_ROWS - 1;
    }

    terminal_move_cursor(new_x, new_y);
}

void terminal_move_cursor(uint16_t px, uint16_t py){
    if (px == terminal_cursor.x && py == terminal_cursor.y) return;

    terminal_redraw_cell(terminal_cursor.x, terminal_cursor.y);
    terminal_cursor.x = px;
    terminal_cursor.y = py;
}

void terminal_render_cursor(void){
    if (!terminal_cursor.visible) return;

    int px = terminal_cursor.x * FONT->width;
    int py = terminal_cursor.y * FONT->height;

    // simple version: invert block
    renderer_fill_rectangle(px, py + FONT->height - 2, 2, FONT->width, terminal_default_fg);
}

void terminal_set_char(uint16_t px, uint16_t py, char c){
    if (px >= TERMINAL_COLS || py >= TERMINAL_ROWS) return;

    terminal_cell_t* cell = terminal_get_cell(px, py);
    cell->c = c;
    cell->fg = terminal_default_fg;
    cell->bg = terminal_default_bg;
    renderer_draw_raw_char(px * FONT->width, py * FONT->height, c, cell->fg, cell->bg);
}

void terminal_set_char_c(uint16_t px, uint16_t py, char c, color_t fg, color_t bg){
    if (px >= TERMINAL_COLS || py >= TERMINAL_ROWS) return;

    terminal_cell_t* cell = terminal_get_cell(px, py);
    cell->c = c;
    cell->fg = color_lut[fg];
    cell->bg = color_lut[bg];
    renderer_draw_raw_char(px * FONT->width, py * FONT->height, c, cell->fg, cell->bg);
}

void terminal_rm_char(uint16_t px, uint16_t py){
    if (px >= TERMINAL_COLS || py >= TERMINAL_ROWS) return;

    terminal_cell_t* cell = terminal_get_cell(px, py);
    cell-> c = ' ';
    cell->fg = terminal_default_fg;     // Probably not strictly necessary since any insertion updates this anyways
    cell->bg = terminal_default_bg;
    renderer_draw_raw_char(px * FONT->width, py * FONT->height, ' ', cell->fg, cell->bg);
}

static void redraw_row(int screen_row) {
    for (int col = 0; col < TERMINAL_COLS; col++) {
        terminal_cell_t* cell = terminal_get_cell(col, screen_row);

        renderer_draw_raw_char(col * FONT->width, screen_row * FONT->height, cell->c, cell->fg, cell->bg);
    }
}

void terminal_scroll(int rows){
    if (rows > TERMINAL_ROWS) rows = TERMINAL_ROWS;
    if (rows < -TERMINAL_ROWS) rows = -TERMINAL_ROWS;

    viewport_start = (viewport_start + rows) % TERMINAL_ROWS;

        if (viewport_start < 0) viewport_start += TERMINAL_ROWS;

    renderer_scroll_vertical(rows * FONT->height);
    
    if (rows > 0) {
        // redraw bottom rows
        for (int r = TERMINAL_ROWS - rows; r < TERMINAL_ROWS; r++) {
            redraw_row(r);
        }
        renderer_fill_rectangle(0, TERMINAL_ROWS * FONT->height - FONT->height * rows, FONT->height * TERMINAL_ROWS, TERMINAL_COLS * FONT->width, terminal_default_bg);
    } else if (rows < 0) {
        // redraw top rows
        for (int r = 0; r < -rows; r++) {
            redraw_row(r);
        }
        renderer_fill_rectangle(0, 0, FONT->height * -rows, TERMINAL_COLS * FONT->width, terminal_default_bg);
    }
}

void terminal_redraw_cell(uint16_t px, uint16_t py){
    terminal_cell_t* cell = terminal_get_cell(px, py);
    renderer_draw_raw_char(px * FONT->width, py * FONT->height, cell->c, cell->fg, cell->bg);
}