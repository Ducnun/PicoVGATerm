#pragma once

#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "hsync.pio.h"

#define HSYNC_PIN   4
#define VSYNC_PIN   5
#define R_MSB_PIN   6
#define R_LSB_PIN   7
#define G_MSB_PIN   8
#define G_LSB_PIN   9
#define B_MSB_PIN   10
#define B_LSB_PIN   11

#define HSYNC_FREQ  31470.0f
#define HSYNC_INSTR_LEN 59

volatile extern uint32_t line_count;

void start_hsync(PIO pio, uint sm, uint offset);
void start_vsync_hook();