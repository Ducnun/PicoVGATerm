#include "vga.h"
#include "hardware/pio.h"

uint8_t* write_ptr = &framebuffer[0];

// Initialize scanline output SM and DMA
void start_vga(PIO pio) {
    // Get the 3 state machines configured with their defaults provided through PIOASM
    uint hsync_offset = pio_add_program(pio, &hsync_program);
    uint vsync_offset = pio_add_program(pio, &vsync_program);
    uint rgb_offset = pio_add_program(pio, &rgb_program);
    pio_sm_claim(pio, HSYNC_SM);
    pio_sm_claim(pio, VSYNC_SM);
    pio_sm_claim(pio, RGB_SM);
    hsync_program_init(pio, HSYNC_SM, hsync_offset, HSYNC_PIN, HSYNC_DIV);
    vsync_program_init(pio, VSYNC_SM, vsync_offset, VSYNC_PIN, VSYNC_DIV);
    rgb_program_init(pio, RGB_SM, rgb_offset, RGB_BASE_PIN, RGB_DIV);

    // Set up DMA channels
    int rgb_data_channel = dma_claim_unused_channel(true);
    int rgb_conf_channel = dma_claim_unused_channel(true);

    // Pixel data DMA channel
    dma_channel_config c0 = dma_channel_get_default_config(rgb_data_channel);
    channel_config_set_transfer_data_size(&c0, DMA_SIZE_8);             // 8-bit txfers
    channel_config_set_read_increment(&c0, true);                       // yes read incrementing
    channel_config_set_write_increment(&c0, false);                     // no write incrementing
    channel_config_set_dreq(&c0, DREQ_PIO0_TX2) ;                       // DREQ_PIO0_TX2 pacing (FIFO)
    channel_config_set_chain_to(&c0, rgb_conf_channel);                 // chain to other channel

    dma_channel_configure(
        rgb_data_channel,           // Channel to be configured
        &c0,                        // The configuration we just created
        &pio->txf[RGB_SM],          // write address (RGB PIO TX FIFO)
        &framebuffer,               // The initial read address (pixel color array)
        TXCOUNT,                    // Number of transfers (per byte)
        false                       // Don't start immediately
    );

    dma_channel_config c1 = dma_channel_get_default_config(rgb_conf_channel);
    channel_config_set_transfer_data_size(&c1, DMA_SIZE_32);        // 32 bit transfers (ptr on 32 bit system)
    channel_config_set_read_increment(&c1, false);                  // Keep read stable
    channel_config_set_write_increment(&c1, false);                 // Keep write stable too
    channel_config_set_chain_to(&c1, rgb_data_channel);             // Chain to other channel

    dma_channel_configure(
        rgb_conf_channel,       // Channel to be configured
        &c1,                    // Partial configuration created earlier
        &dma_hw->ch[rgb_data_channel].read_addr,    // Write address is start of data dma channel
        &write_ptr,             // Read address
        1,                      // only 1 transfer everytime this dma channel is started
        false                   // Don't start immediately
    );

    // Set them blocking and give the state machines the values they need for their large counters
    pio_sm_put_blocking(pio, HSYNC_SM, H_ACTIVE);
    pio_sm_put_blocking(pio, VSYNC_SM, V_ACTIVE);
    pio_sm_put_blocking(pio, RGB_SM, RGB_ACTIVE);

    // Now we start all state machines simultaneously
    pio_enable_sm_mask_in_sync(pio, ((1u << HSYNC_SM) | (1u << VSYNC_SM) | (1u << RGB_SM)));
    dma_channel_start(rgb_data_channel);
}

void render_test() {
    for (uint16_t y = 0; y < SCREEN_HEIGHT; y++) {
        for (uint16_t x = 0; x < SCREEN_WIDTH; x++) {

            uint8_t r = x & 0x07;                 // 3-bit red gradient
            uint8_t g = y & 0x07;                 // 3-bit green gradient
            uint8_t b = (x + y) & 0x07;           // diagonal blue variation

            framebuffer[y * SCREEN_WIDTH + x] =
                (g << 5) | (r << 2) | (b >> 1);   // adjust to your bit layout
        }
    }
}

void render_stripes(){
    for (uint16_t y = 0; y < SCREEN_HEIGHT; y++) {
        for (uint16_t x = 0; x < SCREEN_WIDTH; x++) {

            uint8_t r = (x / 8) & 0x07;   // vertical red bands
            uint8_t g = (y / 8) & 0x07;   // horizontal green bands
            uint8_t b = ((x ^ y) / 8) & 0x07;

            framebuffer[y * SCREEN_WIDTH + x] =
                (g << 5) | (r << 2) | b;
        }
    }
}

void render_planes() {
    static uint8_t bit = 0;
    bit = (bit + 1) & 7;

    for (uint16_t y = 0; y < SCREEN_HEIGHT; y++) {
        for (uint16_t x = 0; x < SCREEN_WIDTH; x++) {

            uint8_t value = (x == y) ? (1 << bit) : 0;

            framebuffer[y * SCREEN_WIDTH + x] = value;
        }
    }
}

void render_explosion() {
    for (uint16_t y = 0; y < SCREEN_HEIGHT; y++) {
        for (uint16_t x = 0; x < SCREEN_WIDTH; x++) {

            uint8_t v = 0;

            if ((x & 1)) v |= 0x01;
            if ((x & 2)) v |= 0x02;
            if ((x & 4)) v |= 0x04;

            framebuffer[y * SCREEN_WIDTH + x] = v;
        }
    }
}

void render_animation() {
    static uint16_t t = 0;
    t++;

    for (uint16_t y = 0; y < SCREEN_HEIGHT; y++) {
        for (uint16_t x = 0; x < SCREEN_WIDTH; x++) {

            int16_t cx = x - (SCREEN_WIDTH / 2);
            int16_t cy = y - (SCREEN_HEIGHT / 2);

            uint16_t dist = (cx * cx + cy * cy) >> 8;

            uint8_t r = (dist + t) & 0x07;
            uint8_t g = ((dist >> 1) + (t >> 1)) & 0x07;
            uint8_t b = ((dist >> 2) + (t >> 2)) & 0x07;

            framebuffer[y * SCREEN_WIDTH + x] =
                (g << 5) | (r << 2) | b;
        }
    }
}

void render_validation() {
    for (uint16_t y = 0; y < SCREEN_HEIGHT; y++) {
        uint8_t bit = (y / 20) & 7;  // each horizontal band = one bit

        for (uint16_t x = 0; x < SCREEN_WIDTH; x++) {

            uint8_t value;

            // alternating stripes to make it obvious
            if ((x / 16) & 1)
                value = (1 << bit);
            else
                value = 0;

            framebuffer[y * SCREEN_WIDTH + x] = value;
        }
    }
}

void render_calibration() {
    for (uint16_t y = 0; y < SCREEN_HEIGHT; y++) {
        for (uint16_t x = 0; x < SCREEN_WIDTH; x++) {

            uint8_t color = 0;

            // --- Top: grayscale gradient ---
            if (y < SCREEN_HEIGHT / 4) {
                uint8_t v = (x * 8) / SCREEN_WIDTH; // 0–7
                color = (v << 5) | (v << 2) | v;
            }

            // --- Second: RGB bars ---
            else if (y < SCREEN_HEIGHT / 2) {
                uint8_t section = (x * 3) / SCREEN_WIDTH;

                if (section == 0) color = 0b00000111;       // Blue
                else if (section == 1) color = 0b00011100;  // Red
                else color = 0b11100000;                    // Green
            }

            // --- Third: color gradient mix ---
            else if (y < 3 * SCREEN_HEIGHT / 4) {
                uint8_t r = (x * 8) / SCREEN_WIDTH;
                uint8_t g = (y * 8) / SCREEN_HEIGHT;
                uint8_t b = ((x + y) * 8) / (SCREEN_WIDTH + SCREEN_HEIGHT);

                color = (g << 5) | (r << 2) | b;
            }

            // --- Bottom: checker contrast test ---
            else {
                uint8_t cell = ((x / 16) ^ (y / 16)) & 1;
                color = cell ? 0xFF : 0x00;
            }

            framebuffer[y * SCREEN_WIDTH + x] = color;
        }
    }
}

// 5x7 font for required characters (H, E, L, O, W, R, D, space)
static const uint8_t font[8][7] = {
    // H
    {0x11,0x11,0x1F,0x11,0x11,0x11,0x11},
    // E
    {0x1F,0x10,0x1E,0x10,0x10,0x10,0x1F},
    // L
    {0x10,0x10,0x10,0x10,0x10,0x10,0x1F},
    // O
    {0x0E,0x11,0x11,0x11,0x11,0x11,0x0E},
    // W
    {0x11,0x11,0x11,0x15,0x15,0x15,0x0A},
    // R
    {0x1E,0x11,0x11,0x1E,0x14,0x12,0x11},
    // D
    {0x1E,0x11,0x11,0x11,0x11,0x11,0x1E},
    // space
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00}
};

enum {
    FONT_H, FONT_E, FONT_L, FONT_O,
    FONT_W, FONT_R, FONT_D, FONT_SPACE
};

const uint8_t text[] = {
    FONT_H, FONT_E, FONT_L, FONT_L, FONT_O,
    FONT_SPACE,
    FONT_W, FONT_O, FONT_R, FONT_L, FONT_D
};

void render_hello_world() {
    // --- clear screen ---
    for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) {
        framebuffer[i] = 0x00;
    }

    int start_x = 20;
    int start_y = 40;

    // 8 different colors (cycle through all bits)
    uint8_t colors[8] = {
        0b00000111, // RED
        0b00011100, // GREEN
        0b11100000, // BLUE
        0b00011111, // cyan-ish
        0b11100111, // yellow-ish
        0b11111100, // magenta-ish
        0b10101010, // mixed pattern
        0b11111111  // white
    };

    for (int c = 0; c < sizeof(text); c++) {
        int char_x = start_x + c * 12; // wider spacing for background
        int char_y = start_y;

        uint8_t bg = colors[c % 8];

        // --- draw background cell (10x10 block) ---
        for (int dy = 0; dy < 10; dy++) {
            for (int dx = 0; dx < 10; dx++) {
                int x = char_x + dx;
                int y = char_y + dy;

                framebuffer[y * SCREEN_WIDTH + x] = bg;
            }
        }

        // --- draw character on top ---
        for (int row = 0; row < 7; row++) {
            uint8_t bits = font[text[c]][row];

            for (int col = 0; col < 5; col++) {
                if (bits & (1 << (4 - col))) {
                    int x = char_x + 2 + col; // center inside cell
                    int y = char_y + 1 + row;

                    framebuffer[y * SCREEN_WIDTH + x] = 0xFF; // white text
                }
            }
        }
    }
}