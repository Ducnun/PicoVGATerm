#pragma once

#include "terminal/terminal_cell.h"
#include "render/renderer.h"

#define TERMINAL_ROWS   60
#define TERMINAL_COLS   80

typedef struct {
    terminal_cell_t* cells;
    uint8_t rows;
    uint8_t cols;
    uint8_t scroll_index;
} screenbuffer_t;

typedef struct {
    uint8_t x;
    uint8_t y;
    bool visible;
} cursor_t;

static cursor_t terminal_cursor = {0, 0, true};

void terminal_set_default_fg(color_t color);
void terminal_set_default_bg(color_t color);
void terminal_print_char(char c);
void terminal_newline();
void terminal_clear();

void terminal_scroll(int rows);
void terminal_set_char(uint16_t px, uint16_t py, char c);
void terminal_set_char_c(uint16_t px, uint16_t py, char c, color_t fg, color_t bg);
void terminal_rm_char(uint16_t px, uint16_t py);