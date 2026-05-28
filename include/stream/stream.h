#pragma once

#include <stdint.h>
#include <hardware/uart.h>
#include "terminal/terminal_core.h"

#define UART_ID uart0
#define BAUD_RATE 1000000
#define UART_TX_PIN     0
#define UART_RX_PIN     1
#define UART_CTS_PIN    26 
#define UART_RTS_PIN    27
#define STREAM_MAX_PAYLOAD 64
#define RX_RING_SIZE 1024

// Packet layout
// [SYNC, CMD, LEN, ARGS[0-63], CRC]

typedef enum {
    STREAM_CMD_SYNC_BYTE    = 0xFF,
    STREAM_CMD_ACK_BYTE     = 0xFE,
    STREAM_CMD_NACK_BYTE    = 0xFD,
    STREAM_CMD_PRINT_CHAR   = 0x01,
    STREAM_CMD_PRINT_STRING = 0x02,
    STREAM_CMD_NEWLINE      = 0x03,
    STREAM_CMD_BACKSPACE    = 0x04,
    STREAM_CMD_REDRAW_LINE  = 0x05,
    STREAM_CMD_SET_CURSOR   = 0x10,
    STREAM_CMD_GET_CURSOR   = 0x11,
    STREAM_CMD_SET_CURSOR_VISIBLE = 0x12,
    STREAM_CMD_SET_FG       = 0x20,
    STREAM_CMD_SET_BG       = 0x21,
    STREAM_CMD_CLEAR        = 0x30,
    STREAM_CMD_SCROLL       = 0x31,
    STREAM_CMD_RESYNC       = 0x80,
} STREAM_CMD;

typedef enum {
    STREAM_RX_STATE_IDLE,
    STREAM_RX_STATE_CMD,
    STREAM_RX_STATE_LEN,
    STREAM_RX_STATE_ARG,
    STREAM_RX_STATE_SEQ,
    STREAM_RX_STATE_CRC,
} STREAM_RX_STATE;

typedef enum {
    STREAM_NACK_CRC = 0x00,
    STREAM_NACK_SEQ = 0x01,
} STREAM_NACK_REASON;

typedef struct {
    STREAM_RX_STATE rx_state;
    STREAM_CMD      cmd;
    uint8_t         length;
    uint8_t         data[STREAM_MAX_PAYLOAD];
    uint8_t         buf_idx;
    bool            seq_initialized;
    uint8_t         prev_seq;
    uint8_t         seq;
    uint8_t         crc;
} stream_state_t;

void stream_uart_init();
void stream_poll_event();
void stream_rx(uint8_t b);
void stream_handle_command();