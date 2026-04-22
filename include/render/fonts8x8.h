#pragma once

#include "render/fonts8x8_bitmaps.h"

typedef struct{
	const uint8_t* bitmap;
	uint8_t height;
	uint8_t width;
} font_t;

// Available fonts
static const font_t FONT_COMPUTER 	= {FONT_COMPUTER_BITMAP, 8, 8};
static const font_t FONT_EVEREST 	= {FONT_EVEREST_BITMAP, 8, 8};
static const font_t FONT_ENVIOUS 	= {FONT_ENVIOUS_BITMAP, 8, 8};

// Global font that is currently used
static const font_t* FONT = &FONT_EVEREST;

static inline const uint8_t* font_get_glyph(char c) {
    uint8_t uc = (uint8_t)c;
    if (uc < 32 || uc > 126) uc = '?';
    return &FONT->bitmap[(uc - 32) * FONT->height];
}

static void set_font(const font_t* f){
	FONT = f;
}