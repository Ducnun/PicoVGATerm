#include "render/renderer.h"

void renderer_draw_char(int px, int py, char c, color_t fg, color_t bg) {
    framebuffer_t fb = get_framebuffer();
    if (px < 0 || py < 0 || px + FONT->width > fb.width || py + FONT->height > fb.height) return;

    if (c == ' ') return renderer_fill_rectangle(px, py, FONT->height, FONT->width, bg);

    const uint8_t* glyph = font_get_glyph(c);

    uint8_t fg_val = color_lut[fg];
    uint8_t bg_val = color_lut[bg];

    for (int row = 0; row < FONT->height; row++) {
        uint8_t bits = glyph[row];
        uint8_t* dst = &fb.data[(py + row) * fb.line_stride + px];

        for (int col = 0; col < FONT->width; col++) {
            dst[col] = (bits & (1 << (7 - col))) ? fg_val : bg_val;
        }
    }
}

void renderer_draw_raw_char(int px, int py, char c, uint8_t fg, uint8_t bg) {
    framebuffer_t fb = get_framebuffer();
    const uint8_t* glyph = font_get_glyph(c);

    for (int row = 0; row < FONT->height; row++) {
        uint8_t bits = glyph[row];
        uint8_t* dst = &fb.data[(py + row) * fb.line_stride + px];

        for (int col = 0; col < FONT->width; col++) {
            dst[col] = (bits & (1 << (7 - col))) ? fg : bg;
        }
    }
}

void renderer_scroll_vertical(int lines){
    framebuffer_t fb = get_framebuffer();
    if (lines > 0){
        memmove(fb.data,
        fb.data + lines * fb.line_stride,
        (fb.height - lines) * fb.line_stride);
    }
    else if (lines < 0){
        memmove(fb.data + lines * fb.line_stride,
        fb.data,
        (fb.height - lines) * fb.line_stride);
    }
}

void renderer_fill_rectangle(int px, int py, int height, int width, color_t color){
    framebuffer_t fb = get_framebuffer();

    // clip (important to avoid memory corruption)
    if (px < 0) {
        width += px;
        px = 0;
    }
    if (py < 0) {
        height += py;
        py = 0;
    }
    if (px + width > fb.width) {
        width = fb.width - px;
    }
    if (py + height > fb.height) {
        height = fb.height - py;
    }

    if (width <= 0 || height <= 0) return;

    uint8_t color_val = color_lut[color];

    for (int row = 0; row < height; row++) {
        uint8_t* dst = &fb.data[(py + row) * fb.line_stride + px];

        memset(dst, color_val, width);
    }
}