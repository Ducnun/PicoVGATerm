#include "render/renderer.h"

void draw_char(framebuffer_t fb, int px, int py, char c, color_t fg, color_t bg) {
    if (px < 0 || py < 0 || px + FONT->width > fb.line_stride || py + FONT->height > fb.height) return;

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

static inline void draw_cell(framebuffer_t fb, uint8_t col, uint8_t row, terminal_cell_t* cell){
    const uint8_t* glyph = font_get_glyph(cell->c);
    uint16_t px = col * FONT->width;
    uint16_t py = row * FONT->height;
    for (int row = 0; row < FONT->height; row++) {
        uint8_t bits = glyph[row];
        uint8_t* dst = &fb.data[(py + row) * fb.line_stride + px];

        for (int col = 0; col < FONT->width; col++) {
            dst[col] = (bits & (1 << (7 - col))) ? cell->fg : cell->bg;
        }
    }
}

void render_terminal(framebuffer_t fb){
    screenbuffer_t buffer = get_screenbuffer();
    for (int cur_row = 0; cur_row < buffer.rows; cur_row++){
        for (int cur_col = 0; cur_col < buffer.cols; cur_col++){
            
            uint8_t scroll_adjusted_row = cur_row + buffer.scroll_index;
            terminal_cell_t* cell = &buffer.cells[scroll_adjusted_row * buffer.cols + cur_col];

            // only render changed cells
            if (cell->flags & CELL_FLAG_DIRTY) {
                draw_cell(fb, 
                    cur_col, 
                    scroll_adjusted_row, 
                    cell);   

                cell->flags &= ~CELL_FLAG_DIRTY;
            }
        }
    }
}