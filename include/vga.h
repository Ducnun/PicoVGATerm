#pragma once

#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "hsync.pio.h"
#include "vga_timings.h"

#ifndef CUSTOM_VGA_PINS
    #define HSYNC_PIN   5
    #define VSYNC_PIN   4
    #define R_MSB_PIN   6
    #define R_LSB_PIN   7
    #define G_MSB_PIN   8
    #define G_LSB_PIN   9
    #define B_MSB_PIN   10
    #define B_LSB_PIN   11
#endif

#define HSYNC_INSTR_LEN 59

volatile const vga_timing_t* timing;
volatile extern uint32_t line_count;

void start_hsync(PIO pio, uint sm, uint offset);
void start_vsync_hook();