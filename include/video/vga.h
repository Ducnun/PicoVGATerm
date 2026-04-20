#pragma once

#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "hardware/dma.h"

#include "video/hsync.pio.h"
#include "video/vsync.pio.h"
#include "video/rgb.pio.h"
#include "video/framebuffer.h"

// Definitions for rendering
#define SCREEN_WIDTH    640
#define SCREEN_HEIGHT   480
#define LINE_STRIDE     (640 + 1)   // 1 additional byte for blanking

// Definitions for signal generation
#define PIXEL_CLOCK     25000000
#define RGB_ACTIVE      639
#define H_ACTIVE        655         // Active and front porch - 1
#define V_ACTIVE        479         // Active -1
#define TXCOUNT         LINE_STRIDE * SCREEN_HEIGHT  // A +1 here in width for stuffing a blanking byte into the DMA stream
#define HSYNC_DIV       ((float)SYS_CLK_HZ / PIXEL_CLOCK)
#define VSYNC_DIV       HSYNC_DIV
#define RGB_DIV         (float)HSYNC_DIV / 5.0f

// State machine division accross PIO block
#define HSYNC_SM    0
#define VSYNC_SM    1
#define RGB_SM      2

uint8_t framebuffer[TXCOUNT];

#ifndef CUSTOM_VGA_PINS
    #define HSYNC_PIN   15
    #define VSYNC_PIN   14
    #define R_MSB_PIN   2
    #define R_MID_PIN   3
    #define R_LSB_PIN   4
    #define G_MSB_PIN   5
    #define G_MID_PIN   6
    #define G_LSB_PIN   7
    #define B_MSB_PIN   8
    #define B_LSB_PIN   9
#endif

#define RGB_BASE_PIN    R_MSB_PIN

/*
    @brief Takes the entire PIO block (needs 3 of the 4 state machines) and combines them
    to make a full VGA signal
    @param pio block used for signal generation
*/
void start_vga(PIO pio);

framebuffer_t get_framebuffer(void);