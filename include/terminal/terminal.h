#pragma once

#include "terminal/terminal_cell.h"
#include "render/colors.h"

#define TERMINAL_ROWS   60
#define TERMINAL_COLS   80

typedef struct {
    terminal_cell_t* cells;
    uint8_t rows;
    uint8_t cols;
} screenbuffer_t;

screenbuffer_t get_screenbuffer();
void set_default_fg(color_t color);
void set_default_bg(color_t color);
void set_char(uint16_t px, uint16_t py, char c);
void set_char_c(uint16_t px, uint16_t py, char c, color_t fg, color_t bg);
void rm_char(uint16_t px, uint16_t py);
void scroll(int rows);