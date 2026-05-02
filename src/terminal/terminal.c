#include "terminal/terminal.h"
#include "stdio.h"
#include "pico/stdlib.h"

// Terminal data
cursor_t cursor = {0, 0, true};
terminal_cell_t screencells[TERMINAL_COLS * TERMINAL_ROWS] = {' '};
terminal_line_t screenlines[TERMINAL_ROWS];

// Complete terminal struct
terminal_t terminal = {
    TERMINAL_COLS, 
    TERMINAL_ROWS,
    0,
    &cursor,
    &screencells,
    &screenlines,
    color_lut[WHITE],
    color_lut[BLACK]
};

static inline terminal_cell_t* terminal_get_cell(uint16_t px, uint16_t py){
    return terminal.cells + ((terminal.viewport_start + py) % terminal.rows * terminal.cols + px);
}

// TEMP Debugging func
void print_cursor_info(){
    printf("Cursor pos: %u, %u \n", terminal.cursor->y, terminal.cursor->x);
    printf("Char at cursor: %c \n", terminal_get_cell(terminal.cursor->x, terminal.cursor->y)->c);
}

void terminal_set_default_fg(color_t color){
    terminal.default_fg = color_lut[color];
}

void terminal_set_default_bg(color_t color){
    terminal.default_bg = color_lut[color];
}

void terminal_print_char(char c){
    terminal_set_char(terminal.cursor->x, terminal.cursor->y, c);

    // Update line end
    if (terminal.cursor->x > terminal.lines[terminal.cursor->y].length){
        terminal.lines[terminal.cursor->y].length = terminal.cursor->x + 1;
    }
    print_cursor_info();
    printf("Updated line length: %d\n", terminal.lines[terminal.cursor->y].length);

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
    // Nothing to do at absolute top-left
    printf("Start of backspace\n");
    if (terminal.cursor->x == 0 && terminal.cursor->y == 0){
        return;
    }

    print_cursor_info();

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
        printf("Wrapped around\n");
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

void terminal_render_cursor(void){
    if (!terminal.cursor->visible) return;

    int px = terminal.cursor->x * FONT->width;
    int py = terminal.cursor->y * FONT->height;

    // simple version: invert block
    renderer_fill_rectangle(px, py + FONT->height - 2, 2, FONT->width, terminal.default_fg);
}

void terminal_set_char(uint16_t px, uint16_t py, char c){
    if (px >= terminal.cols || py >= terminal.rows) return;

    terminal_cell_t* cell = terminal_get_cell(px, py);
    cell->c = c;
    cell->fg = terminal.default_fg;
    cell->bg = terminal.default_bg;
    renderer_draw_raw_char(px * FONT->width, py * FONT->height, c, cell->fg, cell->bg);
}

void terminal_set_char_c(uint16_t px, uint16_t py, char c, color_t fg, color_t bg){
    if (px >= terminal.cols || py >= terminal.rows) return;

    terminal_cell_t* cell = terminal_get_cell(px, py);
    cell->c = c;
    cell->fg = color_lut[fg];
    cell->bg = color_lut[bg];
    renderer_draw_raw_char(px * FONT->width, py * FONT->height, c, cell->fg, cell->bg);
}

void terminal_rm_char(uint16_t px, uint16_t py){
    if (px >= terminal.cols || py >= terminal.rows) return;

    terminal_cell_t* cell = terminal_get_cell(px, py);
    cell-> c = ' ';
    cell->fg = terminal.default_fg;     // Probably not strictly necessary since any insertion updates this anyways
    cell->bg = terminal.default_bg;
    renderer_draw_raw_char(px * FONT->width, py * FONT->height, ' ', cell->fg, cell->bg);
}

static void redraw_row(int screen_row) {
    for (int col = 0; col < terminal.cols; col++) {
        terminal_cell_t* cell = terminal_get_cell(col, screen_row);

        renderer_draw_raw_char(col * FONT->width, screen_row * FONT->height, cell->c, cell->fg, cell->bg);
    }
}

void terminal_scroll(int rows){
    if (rows > terminal.rows) rows = terminal.rows;
    if (rows < -terminal.rows) rows = -terminal.rows;

    terminal.viewport_start = (terminal.viewport_start + rows + terminal.rows) % terminal.rows;

    renderer_scroll_vertical(rows * FONT->height);
    
    if (rows > 0) {
        // redraw bottom rows
        for (int r = terminal.rows - rows; r < terminal.rows; r++) {
            redraw_row(r);
        }
        renderer_fill_rectangle(0, terminal.rows * FONT->height - FONT->height * rows, FONT->height * terminal.rows, terminal.cols * FONT->width, terminal.default_bg);
    } else if (rows < 0) {
        // redraw top rows
        for (int r = 0; r < -rows; r++) {
            redraw_row(r);
        }
        renderer_fill_rectangle(0, 0, FONT->height * -rows, terminal.cols * FONT->width, terminal.default_bg);
    }
}

void terminal_redraw_cell(uint16_t px, uint16_t py){
    terminal_cell_t* cell = terminal_get_cell(px, py);
    printf("Redrawing cell at: %u, %u\n", terminal.cursor->y, terminal.cursor->x);
    printf("Cell value is now: %c\n", cell->c);
    renderer_draw_raw_char(px * FONT->width, py * FONT->height, cell->c, cell->fg, cell->bg);
}