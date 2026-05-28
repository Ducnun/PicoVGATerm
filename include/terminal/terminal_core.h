#pragma once

#include "terminal/terminal_types.h"
#include "render/renderer.h"

#define TERMINAL_ROWS   60
#define TERMINAL_COLS   80

const terminal_t* terminal_get_const(void);
const terminal_cell_t* terminal_get_cell_const(uint16_t px, uint16_t py);

void terminal_init(void);
void terminal_set_default_fg(color_t color);
void terminal_set_default_bg(color_t color);
void terminal_print_char(char c);
void terminal_backspace(void);
void terminal_newline(void);
void terminal_render_cursor(void);
void terminal_move_cursor(uint16_t px, uint16_t py);
void terminal_clear(void);

void terminal_scroll(int rows);
void terminal_set_char(uint16_t px, uint16_t py, char c);
void terminal_set_char_c(uint16_t px, uint16_t py, char c, color_t fg, color_t bg);
void terminal_rm_char(uint16_t px, uint16_t py);
void terminal_redraw_cell(uint16_t px, uint16_t py);
