#pragma once

#include <stdint.h>
#include "terminal/terminal.h"

typedef enum {
    STREAM_CMD_PRINT_CHAR = 0x01,
    STREAM_CMD_PRINT_STRING = 0x02,
    STREAM_CMD_NEWLINE = 0x03,
    STREAM_CMD_BACKSPACE = 0x04,
    STREAM_CMD_SET_CURSOR = 0x10,
    STREAM_CMD_GET_CURSOR = 0x11,
    STREAM_CMD_SET_CURSOR_VISIBLE = 0x12,
    STREAM_CMD_SET_FG = 0x20,
    STREAM_CMD_SET_BG = 0x21,
    STREAM_CMD_CLEAR = 0x30,
    STREAM_CMD_SCROLL = 0x31,
} STREAM_CMD;

typedef enum {
    STREAM_RX_STATE_CMD,
    STREAM_RX_STATE_LEN,
    STREAM_RX_STATE_ARG,
} STREAM_RX_STATE;

static STREAM_RX_STATE stream_rx_state = STREAM_RX_STATE_CMD;
static STREAM_CMD stream_current_cmd;
static uint16_t stream_current_cmd_length = 0;
static uint8_t stream_cmd_buffer[8];
static uint8_t stream_cmd_buffer_index = 0;

void stream_rx(uint8_t b);
void stream_handle_command();