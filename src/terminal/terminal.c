#include "terminal/terminal.h"

static terminal_cell_t screencells[TERMINAL_COLS * TERMINAL_ROWS];
static uint8_t viewport_start = 0;
static uint8_t terminal_default_fg = color_lut[WHITE];
static uint8_t terminal_default_bg = color_lut[BLACK];

void terminal_set_default_fg(color_t color){
    terminal_default_fg = color_lut[color];
}

void terminal_set_default_bg(color_t color){
    terminal_default_bg = color_lut[color];
}

void terminal_print_char(char c){
    terminal_set_char(terminal_cursor.x, terminal_cursor.y, c);
    terminal_cursor.x++;

    // Horizontal wrap
    if (terminal_cursor.x >= TERMINAL_COLS){
        terminal_cursor.x = 0;
        terminal_cursor.y++;
    }
    // Vertical wrap
    if (terminal_cursor.y >= TERMINAL_ROWS){
        terminal_scroll(1);
        terminal_cursor.y = TERMINAL_ROWS - 1;
    }
}

void terminal_newline(){
    terminal_cursor.x = 0;
    terminal_cursor.y++;

    // Vertical wrap
    if (terminal_cursor.y >= TERMINAL_ROWS){
        terminal_scroll(1);
        terminal_cursor.y = TERMINAL_ROWS - 1;
    }
}

void terminal_set_char(uint16_t px, uint16_t py, char c){
    if (px >= TERMINAL_COLS || py >= TERMINAL_ROWS) return;

    terminal_cell_t* cell = &screencells[py * TERMINAL_COLS + px];
    cell->c = c;
    cell->fg = terminal_default_fg;
    cell->bg = terminal_default_bg;
    renderer_draw_raw_char(px * FONT->width, py * FONT->height, c, cell->fg, cell->bg);
}

void terminal_set_char_c(uint16_t px, uint16_t py, char c, color_t fg, color_t bg){
    if (px >= TERMINAL_COLS || py >= TERMINAL_ROWS) return;

    terminal_cell_t* cell = &screencells[py * TERMINAL_COLS + px];
    cell->c = c;
    cell->fg = color_lut[fg];
    cell->bg = color_lut[bg];
    renderer_draw_raw_char(px * FONT->width, py * FONT->height, c, cell->fg, cell->bg);
}

void terminal_rm_char(uint16_t px, uint16_t py){
    if (px >= TERMINAL_COLS || py >= TERMINAL_ROWS) return;

    terminal_cell_t* cell = &screencells[py * TERMINAL_COLS + px];
    cell-> c = ' ';
    cell->fg = terminal_default_fg;     // Probably not strictly necessary since any insertion updates this anyways
    cell->bg = terminal_default_bg;
    renderer_draw_raw_char(px * FONT->width, py * FONT->height, ' ', cell->fg, cell->bg);
}

static void redraw_row(int screen_row) {
    int logical_row =
        (viewport_start + screen_row) % TERMINAL_ROWS;

    for (int col = 0; col < TERMINAL_COLS; col++) {
        terminal_cell_t* cell =
            &screencells[logical_row * TERMINAL_COLS + col];

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