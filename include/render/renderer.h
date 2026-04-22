#pragma once

#include <memory.h>
#include "video/vga.h"
#include "render/fonts8x8.h"
#include "render/colors.h"

/**
 * @brief Draws a char at the desired location in the given framebuffer struct
 * @param fb Framebuffer struct that contains ptr to the data, height and width
 * @param px X coordinate of start pixel to draw char at
 * @param py Y coordinate of start pixel to draw char at
 * @param c Character to draw
 * @param fg Foreground color (text colour)
 * @param bg Background color of the cell
 */
void renderer_draw_char(int px, int py, char c, color_t fg, color_t bg);
void renderer_draw_raw_char(int px, int py, char c, uint8_t fg, uint8_t bg);
void renderer_scroll_vertical(int lines);
void renderer_fill_rectangle(int px, int py, int height, int width, color_t color);