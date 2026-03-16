#include "vga_timings.h"

const vga_timing_t vga_640x480 = {
    .visible_lines      = 480,
    .total_lines        = 525,
    .visisble_columns   = 640,
    .total_columns      = 800,
    .vsync_start        = 490,
    .vsync_end          = 492,
    .vfreq              = 59.94 
};

const vga_timing_t vga_320x200 = {
    .visible_lines      = 200,
    .total_lines        = 206,
    .visisble_columns   = 320,
    .total_columns      = 400,
    .vsync_start        = 202,
    .vsync_end          = 204,
    .vfreq              = 59.94
};