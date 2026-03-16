#pragma once

#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "hardware/dma.h"

#include "hsync.pio.h"
#include "scanline_out.pio.h"
#include "vga_timings.h"

#define CELL    8

#ifndef CUSTOM_VGA_PINS
    #define HSYNC_PIN   15
    #define VSYNC_PIN   14
    #define R_MSB_PIN   0
    #define R_MID_PIN   1
    #define R_LSB_PIN   2
    #define G_MSB_PIN   3
    #define G_MID_PIN   4
    #define G_LSB_PIN   5
    #define B_MSB_PIN   6
    #define B_LSB_PIN   7
#endif

#define HSYNC_INSTR_LEN     59
#define SCANLINE_INSTR_LEN  1
#define PIXEL_DMA_CHANNEL   1

volatile const vga_timing_t* timing;
volatile extern uint32_t line_count;
volatile extern uint8_t scanline[800];  // Visible + front porch + sync + back porch

void start_hsync(PIO pio, uint sm, uint offset);
void start_scanline_out(PIO pio, uint sm, uint offset);
void start_vsync_hook();
void render_scanline();