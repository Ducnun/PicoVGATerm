## Abstract
The rendering subsystem is responsible for translating the logical state of the terminal into pixels displayed on the screen. This rendering stage fits between the uart receive and terminal state mutation and the VGA driver that drives the actual signal. To maintain high framerates on the resource constrained RP2350, the terminal state hints at what parts of the screen need to be rendered. These so called dirty cells are then rendered in the flush function by calling the right renderer functions that draws the pixels into the framebuffer.

## Introduction
The rendering pipeline acts as the final stage of the terminal emulator. It interprets a shared `terminal_t` structure mutated at the communication layer (as described in [UART terminal](UART_terminal.md)). It does so by reading the state of this shared terminal and performing a render pass, or flush, that calls functions of the renderer. The renderer in turn writes pixel data directly into `framebuffer_t`, which is output directly by the VGA driver.

## Problem definition
The main challenge in rendering on constrained hardware is the speed at which the CPU can write pixels into the framebuffer compared to the speed of instructions coming in.

### Requirements
- **R1: Incremental updates:** Only redraw what has changed to conserve CPU cycles
- **R2: Efficient erasing:** Clear trailing characters when lines shorten and on scrolling without redrawing full lines
- **R3: Color**: The terminal should support at least 16 colors with simple look up operations
- **R4: Visual Feedback**: Provide a cursor to show where the next character will be written to

## Design
### Flush pipeline
The `terminal_flush()` function serves as the synchronization point between the logical state in `terminal_t` and framebuffer in `framebuffer_t`. It utilizes two flags per line (`LINE_FLAG_PARTIAL_DIRTY`, `LINE_FLAG_FULL_DIRTY`). The full line redraw flag is set when scrolling or clearing. These both indicate that the whole line has been updated or filled with data. On single character insertion or removal, the partial dirty flag is set to tell the flush function to check per cell whether it is dirty. This means more look ups compared to a full dirty line, but less rendering calls as only those dirty cells are actually redrawn. Because of this trade off, both are implemented and used to try and optimize for the quickest option in both scenarios.

### Trailing text (Cache)
One feature of the renderer is the cache. This allows for the renderer to compare characteristics of the previously drawn terminal state with the new one and see if shortcuts are possible. One of these shortcuts is comparing the rendered length of text. If the current line is shorter than the previous line, rather than clearing the full line, only the new characters are drawn. Then, from the last character on the line onwards, a black square can be drawn directly over the rest of the frame to ensure any characters in the framebuffer are cleared. If the new logical line is longer than the previous one, nothing needs to be cleared explicitly.

## Implementation
### Drawing glyphs
Characters are rendered using an 8x8 bitmap font. The `renderer_draw_char` functions uses the character thats passed as parameter as index in an array of 8 byte bitmaps. 
```C++
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
```
One important optimization here is to "reroute" any `renderer_draw_char` calls into a `renderer_fill_rectange` call if the character to be drawn is empty. This both reflects the actual state of an empty character and is also much faster than fetching an empty bitmap and performing a nested for loop that maps bits to bytes in the framebuffer and expands them into full colour.

The lookup by `font_get_glyph(c)` returns an array of 8 bytes, where every byte is one 8 pixel row, 1 bit mapping to 1 pixel. Ternary operations are used based on that bit to either write the foreground color or background color previously fetched into that pixel. 
```C++
static inline const uint8_t* font_get_glyph(char c) {
    uint8_t uc = (uint8_t)c;
    if (uc < 32 || uc > 126) uc = '?';
    return &FONT->bitmap[(uc - 32) * FONT->height];
}

static const uint8_t FONT_COMPUTER_BITMAP[] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //   
	0x18, 0x18, 0x18, 0x18, 0x18, 0x00, 0x18, 0x00, // ! 
	0x6c, 0x6c, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00, // " 
    ...
}
```
The font is globally selected. If the fetched value falls outside of the supported list of characters or is not printable, it defaults to a question mark '?'. 

### Color mapping
To simplify the API of the renderer, a `color_t` enum is defined as well as a `color_lut` array. These are linked in indexing, meaning the any value of `color_t` maps to a single byte in the `color_lut` array. This contains 16 values. There is another option for drawing characters with custom mixed colours, which is slightly faster given there is no lookup and gives slightly more control as there are a total of 256 values for colour.

### Optimization
The main optimizations are in the render path of `terminal_render`. In 
```C++
if (line->flags & LINE_FLAG_FULL_DIRTY){
    for (int x = 0; x < terminal->cols; x++) {
        const terminal_cell_t* cell = terminal_get_cell_const(x, y);
        terminal_clear_cell_flags(x, y, CELL_FLAG_DIRTY);
        renderer_draw_char(x * FONT->width,y * FONT->height,cell->c,cell->fg,cell->bg);
    }
    if (line->length < cache.rendered_length[y]) {
        renderer_fill_rectangle(
            line->length * FONT->width,
            y * FONT->height,
            FONT->height,
            (terminal->cols - line->length) * FONT->width,
            terminal->default_bg
        );
    }
}
```
There are two blocks of code executed per line. The first block is for a full dirty line. For these lines, no further checks have to be performed. Instead, the cell is fetched, the dirty flag is cleared from the cell and a `renderer_draw_char()` call is performed to draw the character in the cell.

Afterwards a check is performed to see if there might be any tracing characters in the framebuffer. This is done by checking the cache. If the old length of the line was longer, a black square is drawn over those trailing characters by calling `renderer_fill_rectangle`.

```C++
else if (line->flags & LINE_FLAG_PARTIAL_DIRTY){
    // Take cursor into account
    for (int x = 0; x < terminal->cols; x++) {
        const terminal_cell_t* cell = terminal_get_cell_const(x, y);
        if (cell->flags & CELL_FLAG_DIRTY){
            renderer_draw_char(x * FONT->width,y * FONT->height,cell->c,cell->fg,cell->bg);
            terminal_clear_cell_flags(x, y, CELL_FLAG_DIRTY);
        }
    }
    if (line->length < cache.rendered_length[y]) {
        renderer_fill_rectangle(
            line->length * FONT->width,
            y * FONT->height,
            FONT->height,
            (terminal->cols - line->length) * FONT->width,
            terminal->default_bg
        );
    }
}
```
The second option is that the `LINE_FLAG_PARTIAL_DIRTY` was set. If this is the case, a check is performed for every individual cell to see whether its dirty and needs to be redrawn. Only if that is the case is draw_char called.

## Future improvements
### Hardware-Accelerated Scrolling
Currently, a terminal scroll event sets `LINE_FLAG_FULL_DIRTY` across all rows, forcing the CPU to re-render every visible character from scratch. This creates a significant processing spike. 

The planned improvement is to decouple logical scrolling from physical scrolling by utilizing the existing `renderer_scroll_vertical()` function. When a scroll occurs:
1. The logical `viewport_start` is incremented.
2. The renderer executes a single `memmove()` (or later even a DMA transfer) to shift the physical framebuffer pixels up by `FONT->height` rows.
3. Only the newly exposed bottom row is marked as `LINE_FLAG_FULL_DIRTY` and passed to `renderer_draw_char()`.

This will reduce the scrolling rendering complexity from $O(Rows \times Cols)$ to $O(Cols)$.