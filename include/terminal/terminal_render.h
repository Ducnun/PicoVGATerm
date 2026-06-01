#pragma once

#include "terminal/terminal_core.h"
#include "render/renderer.h"
#include "hardware/timer.h"

typedef struct{
    uint16_t rendered_length[TERMINAL_ROWS];
} terminal_render_cache_t;

void terminal_flush();
void terminal_render_cursor();