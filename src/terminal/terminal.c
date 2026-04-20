#include "terminal/terminal.h"

static terminal_cell_t screencells[TERMINAL_COLS * TERMINAL_ROWS];
static uint8_t scroll_index = 0;
static uint8_t terminal_default_fg = color_lut[WHITE];
static uint8_t terminal_default_bg = color_lut[BLACK];

screenbuffer_t get_screenbuffer(){
    screenbuffer_t buffer;
    buffer.cells = &screencells[0];
    buffer.rows = TERMINAL_ROWS;
    buffer.cols = TERMINAL_COLS;
    buffer.scroll_index = scroll_index;
    return buffer;
}

void set_default_fg(color_t color){
    terminal_default_fg = color_lut[color];
}

void set_default_bg(color_t color){
    terminal_default_bg = color_lut[color];
}

void set_char(uint16_t px, uint16_t py, char c){
    if (px >= TERMINAL_COLS || py >= TERMINAL_ROWS) return;

    terminal_cell_t* cell = &screencells[py * TERMINAL_COLS + px];
    cell->c = c;
    cell->fg = terminal_default_fg;
    cell->bg = terminal_default_bg;
    renderer_draw_raw_char(px * FONT->width, py * FONT->height, c, cell->fg, cell->bg);
}

void set_char_c(uint16_t px, uint16_t py, char c, color_t fg, color_t bg){
    if (px >= TERMINAL_COLS || py >= TERMINAL_ROWS) return;

    terminal_cell_t* cell = &screencells[py * TERMINAL_COLS + px];
    cell->c = c;
    cell->fg = color_lut[fg];
    cell->bg = color_lut[bg];
    renderer_draw_raw_char(px * FONT->width, py * FONT->height, c, cell->fg, cell->bg);
}

void rm_char(uint16_t px, uint16_t py){
    if (px >= TERMINAL_COLS || py >= TERMINAL_ROWS) return;

    terminal_cell_t* cell = &screencells[py * TERMINAL_COLS + px];
    cell-> c = ' ';
    cell->fg = terminal_default_fg;     // Probably not strictly necessary since any insertion updates this anyways
    cell->bg = terminal_default_bg;
    renderer_draw_raw_char(px * FONT->width, py * FONT->height, ' ', cell->fg, cell->bg);
}