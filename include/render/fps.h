#pragma once

#include <hardware/timer.h>
#include <pico/time.h>
#include "render/renderer.h"

void draw_fps(framebuffer_t fb, int fps);
void show_fps(framebuffer_t fb);