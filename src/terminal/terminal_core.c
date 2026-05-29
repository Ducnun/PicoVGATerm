#include "terminal/terminal_core.h"
#include "stdio.h"
#include "pico/stdlib.h"

// Terminal data
cursor_t cursor = {0, 0, true};
terminal_cell_t screencells[TERMINAL_COLS * TERMINAL_ROWS];
terminal_line_t screenlines[TERMINAL_ROWS];

// Complete terminal state
terminal_t terminal = {
    TERMINAL_COLS, 
    TERMINAL_ROWS,
    0,
    &cursor,
    screencells,
    screenlines,
    WHITE,
    BLACK
};

/*
** Global getters
*/
const terminal_t* terminal_get_const(void){
    return &terminal;
}

const terminal_cell_t* terminal_get_cell_const(uint16_t px, uint16_t py){
    return terminal.cells + py + px * terminal.cols;
}

/*
** Local getters and setters
*/

static inline terminal_cell_t* terminal_get_cell_mut(uint16_t px, uint16_t py){
    return terminal.cells + ((terminal.viewport_start + py) % terminal.rows * terminal.cols + px);
}

static inline void terminal_mark_row_dirty(int row){
    if (row < 0 || row >= terminal.rows) return;
    terminal.lines[row].flags |= LINE_FLAG_DIRTY;
}

/*
** Terminal logic functions
*/

void terminal_init(void){
    terminal.viewport_start = 0;

    for (int y = 0; y < terminal.rows; y++){
        terminal.lines[y].length = 0;

        for (int x = 0; x < terminal.cols; x++){
            terminal_cell_t* cell = terminal_get_cell_mut(x, y);
            cell->c  = ' ';
            cell->fg = terminal.default_fg;
            cell->bg = terminal.default_bg;
        }
    }

    // Now actually draw it
    for (int y = 0; y < terminal.rows; y++){
        for (int x = 0; x < terminal.cols; x++){
            terminal_redraw_cell(x, y);
        }
    }
}

void terminal_set_default_fg(color_t color){
    terminal.default_fg = color;
}

void terminal_set_default_bg(color_t color){
    terminal.default_bg = color;

    for (int y = 0; y < terminal.rows; y++){
        for (int x = 0; x < terminal.cols; x++){
            terminal_cell_t* cell = terminal_get_cell_mut(x, y);

            cell->bg = terminal.default_bg;

            terminal_redraw_cell(x, y);
        }
    }
}

void terminal_print_char(char c){
    terminal_set_char(terminal.cursor->x, terminal.cursor->y, c);

    // Update line end
    if (terminal.cursor->x > terminal.lines[terminal.cursor->y].length){
        terminal.lines[terminal.cursor->y].length = terminal.cursor->x + 1;
    }

    uint16_t new_x = terminal.cursor->x + 1;
    uint16_t new_y = terminal.cursor->y;

    // Horizontal wrap
    if (new_x >= terminal.cols){
        new_x = 0;
        new_y++;
    }
    // Vertical wrap
    if (new_y >= terminal.rows){
        terminal_scroll(1);
        new_y = terminal.rows - 1;
    }
    terminal_move_cursor(new_x, new_y);
}

void terminal_backspace(void){
    if (terminal.cursor->x == 0 && terminal.cursor->y == 0){
        return;
    }

    uint16_t new_x = cursor.x;
    uint16_t new_y = cursor.y;

    if (new_x > 0) {
        new_x--;
        terminal.lines[new_y].length--;
    } else {
        new_y--;

        if (terminal.lines[new_y].length > 0) {
            new_x = terminal.lines[new_y].length - 1;
            terminal.lines[new_y].length--;
        } else {
            new_x = 0;
        }
    }

    // Move cursor FIRST (this also clears old cursor visually)
    terminal_move_cursor(new_x, new_y);

    // Then erase character at new position
    terminal_rm_char(new_x, new_y);
}

void terminal_newline(void){
    uint16_t new_x = 0;
    uint16_t new_y = cursor.y + 1;

    if (new_y >= terminal.rows){
        terminal_scroll(1);
        new_y = terminal.rows - 1;
    }

    terminal_move_cursor(new_x, new_y);
}

void terminal_move_cursor(uint16_t px, uint16_t py){
    if (px == terminal.cursor->x && py == terminal.cursor->y) return;

    terminal_redraw_cell(terminal.cursor->x, terminal.cursor->y);
    terminal.cursor->x = px;
    terminal.cursor->y = py;
}

void terminal_set_char(uint16_t px, uint16_t py, char c){
    if (px >= terminal.cols || py >= terminal.rows) return;

    terminal_cell_t* cell = terminal_get_cell_mut(px, py);
    cell->c = c;
    cell->fg = terminal.default_fg;
    cell->bg = terminal.default_bg;
    terminal_mark_row_dirty(py);
}

void terminal_set_char_c(uint16_t px, uint16_t py, char c, color_t fg, color_t bg){
    if (px >= terminal.cols || py >= terminal.rows) return;

    terminal_cell_t* cell = terminal_get_cell_mut(px, py);
    cell->c = c;
    cell->fg = color_lut[fg];
    cell->bg = color_lut[bg];
    terminal_mark_row_dirty(py);
}

void terminal_rm_char(uint16_t px, uint16_t py){
    if (px >= terminal.cols || py >= terminal.rows) return;

    terminal_cell_t* cell = terminal_get_cell_mut(px, py);
    cell-> c = ' ';
    cell->fg = terminal.default_fg;     // Probably not strictly necessary since any insertion updates this anyways
    cell->bg = terminal.default_bg;
    terminal_mark_row_dirty(py);
}

static void redraw_row(int screen_row) {
    for (int col = 0; col < terminal.cols; col++) {
        terminal_mark_row_dirty(screen_row);
        // terminal_cell_t* cell = terminal_get_cell_mut(col, screen_row);
        // renderer_draw_char(col * FONT->width, screen_row * FONT->height, cell->c, cell->fg, cell->bg);
    }
}

void terminal_scroll(int rows){
    if (rows > terminal.rows) rows = terminal.rows;
    if (rows < -terminal.rows) rows = -terminal.rows;

    terminal.viewport_start = (terminal.viewport_start + rows + terminal.rows) % terminal.rows;

    renderer_scroll_vertical(rows * FONT->height);
    
    if (rows > 0) {
        for (int r = terminal.rows - rows; r < terminal.rows; r++) {

            // Clear logical buffer row
            for (int c = 0; c < terminal.cols; c++) {
                terminal_cell_t* cell = terminal_get_cell_mut(c, r);
                cell->c = ' ';
                cell->fg = terminal.default_fg;
                cell->bg = terminal.default_bg;
            }

            terminal.lines[r].length = 0;
        }

        renderer_fill_rectangle(0, terminal.rows * FONT->height - FONT->height * rows, FONT->height * terminal.rows, terminal.cols * FONT->width, terminal.default_bg);
        // redraw bottom rows
        for (int r = terminal.rows - rows; r < terminal.rows; r++) {
            redraw_row(r);
        }
    } else if (rows < 0) {
        renderer_fill_rectangle(0, 0, FONT->height * -rows, terminal.cols * FONT->width, terminal.default_bg);
        // redraw top rows
        for (int r = 0; r < -rows; r++) {
            redraw_row(r);
        }
    }
}

void terminal_redraw_cell(uint16_t px, uint16_t py){
    terminal_cell_t* cell = terminal_get_cell_mut(px, py);
    terminal_mark_row_dirty(py);
    renderer_draw_char(px * FONT->width, py * FONT->height, cell->c, cell->fg, cell->bg);
}

void terminal_clear(void){
    terminal.cursor->x = 0;
    terminal.cursor->y = 0;
    for (int row = 0; row < terminal.rows; row++){
        for (int col = 0; col < terminal.cols; col++){
            terminal.cells[row + col * terminal.rows].c = ' ';
        }
    }
    renderer_fill_rectangle(0, 0, terminal.rows * FONT->height, terminal.cols * FONT->width, terminal.default_bg);
}
