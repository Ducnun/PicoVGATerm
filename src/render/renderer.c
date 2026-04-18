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

static inline void draw_cell(framebuffer_t fb, uint8_t px, uint8_t py, terminal_cell_t* cell){
    const uint8_t* glyph = font_get_glyph(cell->c);
    for (int row = 0; row < 8; row++) {
        uint8_t bits = glyph[row];
        uint8_t* dst = &fb.data[(py + row) * fb.width + px];

        for (int col = 0; col < 8; col++) {
            dst[col] = (bits & (1 << (7 - col))) ? cell->fg : cell->bg;
        }
    }
}

void render_terminal(framebuffer_t fb){
    screenbuffer_t buffer = get_screenbuffer();
    for (int cur_row = 0; cur_row < buffer.rows; cur_row++){
        for (int cur_col = 0; cur_col < buffer.cols; cur_col++){
            terminal_cell_t* cell = &buffer.cells[cur_row * buffer.cols + cur_col];
            if (cell->flags && CELL_FLAG_DIRTY) continue;   // only render changed cells
            draw_cell(fb, cur_row, cur_col, cell);
        }
    }
}