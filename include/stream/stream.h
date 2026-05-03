#pragma once

#include <stdint.h>
#include <hardware/uart.h>
#include "terminal/terminal.h"

#define UART_ID uart0
#define BAUD_RATE 115200
#define UART_TX_PIN 0
#define UART_RX_PIN 1
#define STREAM_MAX_PAYLOAD 64

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

typedef struct {
    STREAM_RX_STATE rx_state;
    STREAM_CMD cmd;
    uint8_t length;
    uint8_t buffer[STREAM_MAX_PAYLOAD];
    uint8_t buf_idx;
} stream_state_t;

void stream_uart_init();
void stream_poll_event();
void stream_rx(uint8_t b);
void stream_handle_command();