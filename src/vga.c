#include "vga.h"
#include "hardware/pio.h"

volatile uint32_t line_count = 0;
uint scanline_dma_chan;
uint const_dma_chan;

uint8_t scanline[640];               // 1 byte per pixel, RGB 3:3:2
uint32_t line_constants[3] = {16, 96, 48}; // front porch, HSYNC, back porch

volatile const vga_timing_t* timing;

// Initialize scanline output SM and DMA
void start_scanline_out(PIO pio, uint sm, uint offset) {

    pio_sm_config c = scanline_out_program_get_default_config(offset);

    // 1 instruction per pixel
    float pixel_clock = 25.175e6f;
    float clkdiv = (float)clock_get_hz(clk_sys) / pixel_clock;
    sm_config_set_clkdiv(&c, clkdiv);

    // RGB pins
    for (int i = 0; i < 8; i++) pio_gpio_init(pio, i + R_MSB_PIN);
    sm_config_set_out_pins(&c, R_MSB_PIN, 8);
    pio_sm_set_consecutive_pindirs(pio, sm, R_MSB_PIN, 8, true);

    sm_config_set_out_shift(&c, true, true, 8); // autopull 8 bits

    // Pixel DMA (feeds visible pixels)
    scanline_dma_chan = dma_claim_unused_channel(true);
    dma_channel_config pixel_cfg = dma_channel_get_default_config(scanline_dma_chan);
    channel_config_set_transfer_data_size(&pixel_cfg, DMA_SIZE_8);
    channel_config_set_read_increment(&pixel_cfg, true);
    channel_config_set_write_increment(&pixel_cfg, false);
    channel_config_set_dreq(&pixel_cfg, pio_get_dreq(pio, sm, true));

    dma_channel_configure(
        scanline_dma_chan,
        &pixel_cfg,
        &pio->txf[sm],
        scanline,
        640,
        false   // start manually later
    );

    // Constant DMA (front porch, HSYNC, back porch)
    const_dma_chan = dma_claim_unused_channel(true);
    dma_channel_config const_cfg = dma_channel_get_default_config(const_dma_chan);
    channel_config_set_transfer_data_size(&const_cfg, DMA_SIZE_32);
    channel_config_set_read_increment(&const_cfg, true);
    channel_config_set_write_increment(&const_cfg, false);
    channel_config_set_dreq(&const_cfg, DREQ_FORCE);

    dma_channel_configure(
        const_dma_chan,
        &const_cfg,
        &pio->txf[sm],       // OSR
        line_constants,
        3,
        false   // start manually later
    );
    
    pio_sm_set_consecutive_pindirs(pio, sm, HSYNC_PIN, 1, true);
    sm_config_set_sideset_pins(&c, HSYNC_PIN);
    // Initialize SM
    pio_sm_init(pio, sm, offset, &c);
    pio_sm_set_enabled(pio, sm, true);
}

// VSYNC / line counting interrupt
void __isr hsync_irq_handler() {
    pio_interrupt_clear(pio0, 0);  // clear IRQ 0
    line_count++;

    // Start VSYNC pulse (active low)
    if (line_count == timing->vsync_start) {
        gpio_put(VSYNC_PIN, 0);
    }
    // End VSYNC pulse
    else if (line_count == timing->vsync_end) {
        gpio_put(VSYNC_PIN, 1);
    }
    else if (line_count >= timing->total_lines) {
        line_count = 0;
    }

    // Restart pixel DMA for next line
    dma_channel_set_read_addr(scanline_dma_chan, scanline, true);
    dma_channel_set_read_addr(const_dma_chan, line_constants, true);
}

// Initialize VSYNC pin and IRQ
void start_vsync_hook() {
    gpio_init(VSYNC_PIN);
    gpio_set_dir(VSYNC_PIN, GPIO_OUT);
    gpio_put(VSYNC_PIN, 1); // idle high

    irq_set_exclusive_handler(PIO0_IRQ_0, hsync_irq_handler);
    irq_set_enabled(PIO0_IRQ_0, true);
    pio_set_irq0_source_enabled(pio0, pis_interrupt0, true);

    line_count = 0; // explicit reset
}

// Example scanline renderer (checkerboard Brabant)
void render_scanline() {
    uint16_t y = line_count;

    for (uint16_t x = 0; x < 640; x++) {
        if (((x >> 3) ^ (y >> 3)) & 1)
            scanline[x] = 0b00000111; // dark
        else
            scanline[x] = 0b11111111; // bright
    }
}
