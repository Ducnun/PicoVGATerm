#pragma once

#include <stdint.h>

typedef enum{
    BLACK,
    WHITE,
    RED,
    CYAN,
    PURPLE,
    GREEN,
    BLUE,
    YELLOW,
    ORANGE,
    BROWN,
    PINK,
    DARK_GREY,
    MID_GREY,
    LIME,
    VIOLET,
    LIGHT_GREY,
}color_t;

static const uint8_t color_lut[] = {
    0b00000000, // Black
    0b11111111, // White
    0b00000111, // Red
    0b11111000, // Cyan
    0b11100111, // Purple
    0b00111000, // Green
    0b11000000, // Blue
    0b00111111, // Yellow
    0b00010111, // Orange
    0b10010001, // Brown
    0b10100111, // Pink
    0b10100100, // Dark Grey
    0b11101101, // Mid Grey
    0b10111010, // Lime
    0b11110110, // Violet
    0b11011011, // Light grey
};