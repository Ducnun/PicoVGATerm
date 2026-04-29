#include "stream/stream.h"

void stream_rx(uint8_t b){
    switch(stream_rx_state){

        case STREAM_RX_STATE_CMD:
            stream_current_cmd = b;
            stream_rx_state = STREAM_RX_STATE_LEN;
            break;

        case STREAM_RX_STATE_LEN:
            stream_current_cmd_length = b;
            stream_cmd_buffer_index = 0;

            if (b == 0){
                stream_handle_command();
                stream_rx_state = STREAM_RX_STATE_CMD;
            } else {
                stream_rx_state = STREAM_RX_STATE_ARG;
            }
            break;

        case STREAM_RX_STATE_ARG:
            stream_cmd_buffer[stream_cmd_buffer_index++] = b;

            if (stream_cmd_buffer_index >= stream_current_cmd_length){
                stream_handle_command();
                stream_rx_state = STREAM_RX_STATE_CMD;
            }
            break;
    }
}

void stream_handle_command(){
    switch (stream_current_cmd){
        case STREAM_CMD_PRINT_CHAR:
    }
}