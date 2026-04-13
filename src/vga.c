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
    // This is autopulled by the RGB state machine
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

void render_test(){
    const uint8_t color1 = 0b11111111; // red
    const uint8_t color2 = 0b00000000;

    for (uint16_t y = 0; y < SCREEN_HEIGHT; y++) {
        for (uint16_t x = 0; x < SCREEN_WIDTH; x++) {

            // Determine which 16x16 cell we're in
            uint8_t cell_x = x >> 4;  // divide by 16
            uint8_t cell_y = y >> 4;  // divide by 16

            // Checker pattern
            if ((cell_x ^ cell_y) & 1)
                framebuffer[y * SCREEN_WIDTH + x] = color1;
            else
                framebuffer[y * SCREEN_WIDTH + x] = color2;
        }
    }  
}