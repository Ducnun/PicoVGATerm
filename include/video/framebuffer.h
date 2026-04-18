#pragma once

#include <stdint.h>

typedef struct {
    uint8_t* data;
    uint16_t height;
    uint16_t width;
} framebuffer_t;
