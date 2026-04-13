#pragma once

#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "hardware/dma.h"

#include "hsync.pio.h"
#include "vsync.pio.h"
#include "rgb.pio.h"

// Definitions for rendering
#define SCREEN_WIDTH    640
#define SCREEN_HEIGHT   480

// Definitions for signal generation
#define PIXEL_CLOCK     25175000
#define RGB_ACTIVE      639
#define H_ACTIVE        655         // Active and front porch - 1
#define V_ACTIVE        479         // Active -1
#define TXCOUNT         307200      // Total number of pixels (640x480)
#define HSYNC_DIV       ((float)SYS_CLK_HZ / PIXEL_CLOCK)
#define VSYNC_DIV       HSYNC_DIV
#define RGB_DIV         1

// State machine division accross PIO block
#define HSYNC_SM    0
#define VSYNC_SM    1
#define RGB_SM      2

uint8_t framebuffer[TXCOUNT];

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

#define RGB_BASE_PIN    R_MSB_PIN

volatile extern uint32_t line_count;
extern uint8_t scanline[RGB_ACTIVE];  // Visible + front porch + sync + back porch

/*
    @brief Takes the entire PIO block (needs 3 of the 4 state machines) and combines them
    to make a full VGA signal
    @param pio block used for signal generation
*/
void start_vga(PIO pio);
void render_test();
