#include "stream/stream.h"
#include "stdio.h"
#include "pico/stdlib.h"

stream_state_t stream_state;

// UART ring buffer for IRQ
volatile uint8_t rx_ring[RX_RING_SIZE];
volatile uint16_t rx_head = 0;
volatile uint16_t rx_tail = 0;

void on_uart_rx() {
    while (uart_is_readable(UART_ID)) {
        uint8_t b = uart_getc(UART_ID);
        uint16_t next = (rx_head + 1) % RX_RING_SIZE;

        if (next != rx_tail) { // avoid overflow
            rx_ring[rx_head] = b;
            rx_head = next;
        }
    }
}

// UART setup
void stream_uart_init(){
    uart_init(UART_ID, BAUD_RATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
    stream_state.rx_state = STREAM_RX_STATE_CMD;
    stream_state.buf_idx = 0;
    irq_set_exclusive_handler(UART0_IRQ, on_uart_rx);
    irq_set_enabled(UART0_IRQ, true);
    uart_set_irq_enables(UART_ID, true, false);
}

void stream_poll_event(){
    while (rx_tail != rx_head) {
        uint8_t b = rx_ring[rx_tail];
        rx_tail = (rx_tail + 1) % RX_RING_SIZE;
        stream_rx(b);
    }
}

// State machine that parses commands
void stream_rx(uint8_t b){
    printf("cmd: %d\n", stream_state.cmd);
    switch(stream_state.rx_state){

        case STREAM_RX_STATE_CMD:
            stream_state.cmd = b;
            stream_state.rx_state = STREAM_RX_STATE_LEN;
            break;

        case STREAM_RX_STATE_LEN:
            stream_state.length = b;
            stream_state.buf_idx = 0;

            if (b == 0){
                stream_handle_command();
                stream_state.rx_state = STREAM_RX_STATE_CMD;
            } else {
                stream_state.rx_state = STREAM_RX_STATE_ARG;
            }
            break;

        case STREAM_RX_STATE_ARG:
            stream_state.buffer[stream_state.buf_idx++] = b;

            if (stream_state.buf_idx >= stream_state.length){
                stream_handle_command();
                stream_state.rx_state = STREAM_RX_STATE_CMD;
            }
            break;
    }
}

void stream_handle_command(){
    switch (stream_state.cmd){
        case STREAM_CMD_PRINT_CHAR:
            terminal_print_char(stream_state.buffer[0]);
            break;
        case STREAM_CMD_PRINT_STRING:
            for (int i = 0; i < stream_state.length; i++){
                if (stream_state.buffer[i] == '\n') terminal_newline();
                else terminal_print_char(stream_state.buffer[i]);
            }
            break;
        case STREAM_CMD_NEWLINE:
            terminal_newline();
            break;
        case STREAM_CMD_BACKSPACE:
            terminal_backspace();
            break;
        default:
            break;
    }
}