#pragma once

#include <stdint.h>

#define CELL_FLAG_DIRTY (1u << 0)

typedef struct{
    char c;
    uint8_t fg;
    uint8_t bg;
    uint8_t flags;
} terminal_cell_t;