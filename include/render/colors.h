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
    0b00000000,
    0b11111111,
    0b00000111,
    0b11111000,
    0b11100111,
    0b00111000,
    0b11000000,
    0b00111111,
    0b00010111,
    0b10010001,
    0b10100111,
    0b10100100,
    0b11101101,
    0b10111010,
    0b11001001,
    0b11011011,
};