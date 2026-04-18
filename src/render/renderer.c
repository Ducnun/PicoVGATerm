#include "render/renderer.h"

void draw_char(framebuffer_t fb, int px, int py, char c, color_t fg, color_t bg) {
    if (px < 0 || py < 0 || px + 8 > fb.width || py + 8 > fb.height) return;

    const uint8_t* glyph = font_get_glyph(c);

    uint8_t fg_val = color_lut[fg];
    uint8_t bg_val = color_lut[bg];

    for (int row = 0; row < 8; row++) {
        uint8_t bits = glyph[row];
        uint8_t* dst = &fb.data[(py + row) * fb.width + px];

        for (int col = 0; col < 8; col++) {
            dst[col] = (bits & (1 << (7 - col))) ? fg_val : bg_val;
        }
    }
}