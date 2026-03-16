#pragma once

#include <stdint.h>

typedef struct {
    uint16_t visible_lines;
    uint16_t total_lines;
    uint16_t visisble_columns;
    uint16_t total_columns;
    uint16_t vsync_start;
    uint16_t vsync_end;
    float    vfreq;
} vga_timing_t;

extern const vga_timing_t vga_640x480;
extern const vga_timing_t vga_320x200;