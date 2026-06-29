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
** Local getters and setters
*/

static inline terminal_cell_t* terminal_get_cell_mut(uint16_t px, uint16_t py){
    return terminal.cells + ((terminal.viewport_start + py) % terminal.rows * terminal.cols + px);
}

static inline void terminal_mark_cell_dirty(uint16_t px, uint16_t py){
    if (px > terminal.cols || py > terminal.rows) return;
    terminal.lines[py].flags |= LINE_FLAG_PARTIAL_DIRTY;
    terminal_get_cell_mut(px, py)->flags |= CELL_FLAG_DIRTY;
}

static inline void terminal_clear_cell(uint16_t px, uint16_t py){
    terminal_cell_t* cell = terminal_get_cell_mut(px, py);
    cell->c = ' ';
    cell->fg = terminal.default_fg;
    cell->bg = terminal.default_bg;
    cell->flags |= CELL_FLAG_DIRTY;
}

/*
** Global getters and setters
*/
const terminal_t* terminal_get_const(void){
    return &terminal;
}

const terminal_cell_t* terminal_get_cell_const(uint16_t px, uint16_t py){
    return terminal.cells + ((terminal.viewport_start + py) % terminal.rows * terminal.cols + px);
}

void terminal_clear_cell_flags(uint16_t px, uint16_t py, uint8_t mask){
    terminal_get_cell_mut(px, py)->flags &= ~mask;
}

/*
** Terminal high level API
*/

void terminal_init(void){
    terminal.viewport_start = 0;

    terminal.cursor->delay = 500;

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
            terminal_mark_cell_dirty(x, y);
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

            terminal_mark_cell_dirty(x, y);
        }   
    }
}

void terminal_print_char(char c){
    terminal_set_char(terminal.cursor->x, terminal.cursor->y, c);

    // Update line end
    if (terminal.cursor->x >= terminal.lines[terminal.cursor->y].length){
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

/**
** Terminal helper functions
*/

void terminal_move_cursor(uint16_t px, uint16_t py){
    if (px == terminal.cursor->x && py == terminal.cursor->y) return;

    terminal_mark_cell_dirty(terminal.cursor->x, terminal.cursor->y);
    terminal.cursor->x = px;
    terminal.cursor->y = py;
    terminal_mark_cell_dirty(px, py);
}

void terminal_set_char(uint16_t px, uint16_t py, char c){
    if (px >= terminal.cols || py >= terminal.rows) return;

    terminal_cell_t* cell = terminal_get_cell_mut(px, py);
    cell->c = c;
    cell->fg = terminal.default_fg;
    cell->bg = terminal.default_bg;
    terminal_mark_cell_dirty(px, py);
}

void terminal_set_char_c(uint16_t px, uint16_t py, char c, color_t fg, color_t bg){
    if (px >= terminal.cols || py >= terminal.rows) return;

    terminal_cell_t* cell = terminal_get_cell_mut(px, py);
    cell->c = c;
    cell->fg = color_lut[fg];
    cell->bg = color_lut[bg];
    terminal_mark_cell_dirty(px, py);
}

void terminal_rm_char(uint16_t px, uint16_t py){
    if (px >= terminal.cols || py >= terminal.rows) return;
    terminal_clear_cell(px, py);
}

// Scroll that just updates the terminal logical state and does not touch the framebuffer once
void terminal_scroll(int rows){
    if (rows == 0) return;
    if (rows > terminal.rows) rows = terminal.rows;
    if (rows < -terminal.rows) rows = -terminal.rows;

    // Move viewport
    terminal.viewport_start = (terminal.viewport_start + rows + terminal.rows) % terminal.rows;

    // Clear newly exposed rows
    if (rows > 0) {
        // scrolling up
        for (int r = terminal.rows - rows; r < terminal.rows; r++) {
            terminal.lines[r].length = 0;
            terminal.lines[r].flags |= LINE_FLAG_FULL_DIRTY;

            for (int c = 0; c < terminal.cols; c++) {
                terminal_clear_cell(c, r);
            }
        }
    }
    else {
        rows = -rows;

        // scrolling down
        for (int r = 0; r < rows; r++) {
            terminal.lines[r].length = 0;
            terminal.lines[r].flags |= LINE_FLAG_FULL_DIRTY;

            for (int c = 0; c < terminal.cols; c++) {
                terminal_clear_cell(c, r);
            }
        }
    }

    // Mark everything dirty
    for (int r = 0; r < terminal.rows; r++) {
        terminal.lines[r].flags |= LINE_FLAG_FULL_DIRTY;
    }
}

void terminal_clear(void){
    terminal.cursor->x = 0;
    terminal.cursor->y = 0;
    for (int row = 0; row < terminal.rows; row++){
        for (int col = 0; col < terminal.cols; col++){
            terminal_clear_cell(col, row);
        }
        terminal.lines[row].flags |= LINE_FLAG_FULL_DIRTY;
    }
}
