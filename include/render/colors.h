#pragma once

#include <stdint.h>

typedef enum{
    BLACK,
    WHITE,
}color_t;

static const uint8_t color_lut[] = {
    0x00,
    0xff,
};