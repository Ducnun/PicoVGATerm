#include "stream/stream.h"
#include "stdio.h"
#include "pico/stdlib.h"

stream_state_t stream_state;

// UART ring buffer for IRQ
volatile uint8_t rx_ring[RX_RING_SIZE];
volatile uint16_t rx_head = 0;
volatile uint16_t rx_tail = 0;

// Local only functions (predeclaration)
void send_ack();
void send_nack(STREAM_NACK_REASON reason);
void process_state_idle(uint8_t b);
void process_state_cmd(uint8_t b);
void process_state_len(uint8_t b);
void process_state_arg(uint8_t b);
void process_state_seq(uint8_t b);
void process_state_crc(uint8_t b);

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

static inline uint8_t crc8_update(uint8_t crc, uint8_t data) {
    crc ^= data;
    for (int i = 0; i < 8; i++) {
        crc = (crc & 0x80) ? (crc << 1) ^ 0x07 : (crc << 1);
    }
    return crc;
}

// UART setup
void stream_uart_init(){
    uart_init(UART_ID, BAUD_RATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
    stream_state.rx_state = STREAM_RX_STATE_IDLE;
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
    switch(stream_state.rx_state){
        // On a sync byte, we reset and move on to receiving the CMD
        case STREAM_RX_STATE_IDLE:
            process_state_idle(b);
            #ifdef DEBUG_PRINTS
                printf("\nNEW MESSAGE\n");
            #endif
            break;
        
        // Store command and start generating CRC
        case STREAM_RX_STATE_CMD:
            process_state_cmd(b);
            #ifdef DEBUG_PRINTS
                printf("CMD: %d\n", b);
            #endif
            break;

        // Receive length (even no packets with no payload have a length byte)
        case STREAM_RX_STATE_LEN:
            process_state_len(b);      
            #ifdef DEBUG_PRINTS
                printf("LEN: %d\n", b);
            #endif
            break;

        case STREAM_RX_STATE_ARG:
            process_state_arg(b);
            #ifdef DEBUG_PRINTS
                printf("ARG[%d]: %d\n", stream_state.buf_idx, b);
            #endif
            break;

        // Check the sequence byte so no messages were missed
        case STREAM_RX_STATE_SEQ:
            process_state_seq(b);
            #ifdef DEBUG_PRINTS
                printf("SEQ: %d\n", b);
            #endif
            break;
        
        // Handle command if transmitted CRC matches our 
        case STREAM_RX_STATE_CRC:
            process_state_crc(b);
            #ifdef DEBUG_PRINTS
                printf("CRC: %d\n", b);
            #endif
            break;
    }
}

void stream_handle_command(){
    switch (stream_state.cmd){
        case STREAM_CMD_PRINT_CHAR:
            terminal_print_char(stream_state.data[0]);
            break;
        case STREAM_CMD_PRINT_STRING:
            for (int i = 0; i < stream_state.length; i++){
                if (stream_state.data[i] == '\n') terminal_newline();
                else terminal_print_char(stream_state.data[i]);
            }
            break;
        case STREAM_CMD_NEWLINE:
            terminal_newline();
            break;
        case STREAM_CMD_BACKSPACE:
            terminal_backspace();
            break;
        case STREAM_CMD_CLEAR:
            terminal_clear();
            break;
        default:
            break;
    }
}

/*
** State machine helper functions
*/
void process_state_idle(uint8_t b){
    if (b == STREAM_CMD_SYNC_BYTE){
        stream_state.rx_state = STREAM_RX_STATE_CMD;
        stream_state.buf_idx = 0;
        stream_state.length = 0;
        stream_state.crc = 0;
    }
}

void process_state_cmd(uint8_t b){
    stream_state.cmd = b;
    stream_state.crc = crc8_update(0, b);
    stream_state.rx_state = STREAM_RX_STATE_LEN;
}

void process_state_len(uint8_t b){
    stream_state.length = b;
    stream_state.crc = crc8_update(stream_state.crc, b);

    if (b > STREAM_MAX_PAYLOAD) {
        // invalid packet, resync
        stream_state.rx_state = STREAM_RX_STATE_IDLE;
        return;
    }

    stream_state.buf_idx = 0;
    // If length is 0, straight to SEQ check
    if (b == 0){
        stream_state.rx_state = STREAM_RX_STATE_SEQ;
    } else {
        stream_state.rx_state = STREAM_RX_STATE_ARG;
    }
}

void process_state_arg(uint8_t b){
    stream_state.crc = crc8_update(stream_state.crc, b);
    stream_state.data[stream_state.buf_idx++] = b;

    if (stream_state.buf_idx >= stream_state.length){
        stream_state.rx_state = STREAM_RX_STATE_SEQ;
    }
}

void process_state_seq(uint8_t b){
    stream_state.crc = crc8_update(stream_state.crc, b);
    stream_state.seq = b;
    stream_state.rx_state = STREAM_RX_STATE_CRC;
}

void process_state_crc(uint8_t b){
    if (b == stream_state.crc){
        // RESYNC packets bypass sequence validation
        if (stream_state.cmd == STREAM_CMD_RESYNC) {

            stream_state.prev_seq = stream_state.seq;
            stream_state.seq_initialized = true;
            stream_state.rx_state = STREAM_RX_STATE_IDLE;
            return;
        }

        // First packet accepted unconditionally
        if (!stream_state.seq_initialized) {
            stream_state.prev_seq = stream_state.seq;
            stream_state.seq_initialized = true;
        }
        else if ((uint8_t)(stream_state.prev_seq + 1) != stream_state.seq) {

            // Sequence mismatch -> reject packet
            stream_state.rx_state = STREAM_RX_STATE_IDLE;
            return;
        }
        else {
            stream_state.prev_seq = stream_state.seq;
        }

        stream_handle_command();
    }

    stream_state.rx_state = STREAM_RX_STATE_IDLE;
}
