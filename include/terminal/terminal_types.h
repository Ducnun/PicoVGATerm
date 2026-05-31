#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "render/colors.h"

// Cell flags
#define CELL_FLAG_DIRTY     (1 << 0)

// Line flags
#define LINE_FLAG_WRAPPED   (1 << 0)
#define LINE_FLAG_DIRTY     (1 << 1)
#define LINE_FLAG_CLEARED   (1 << 2)

// Standard terminal cell datafields, location is relative to the array and thus not stored
typedef struct{
    char        c;
    color_t     fg;
    color_t     bg;
    uint8_t     flags;
} terminal_cell_t;

// Information per terminal line, for better cursor movements
typedef struct{
    uint16_t    length;
    uint8_t     flags;
} terminal_line_t;

// Cursor struct
typedef struct{
    uint8_t     x;
    uint8_t     y;
    bool        visible;
} cursor_t;

typedef struct{
    uint16_t            cols;
    uint16_t            rows;
    uint16_t            viewport_start;
    cursor_t*           cursor;
    terminal_cell_t*    cells;
    terminal_line_t*    lines;
    color_t             default_fg;
    color_t             default_bg; 
} terminal_t;