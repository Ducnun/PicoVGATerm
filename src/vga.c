#include "vga.h"

volatile uint32_t line_count = 0;
volatile uint8_t scanline[800] = {0};
uint scanline_dma_channel;

void start_hsync(PIO pio, uint sm, uint offset){
    pio_sm_config c = hsync_program_get_default_config(offset);

    // Calculate and set clock divider
    float hfreq = timing->vfreq * timing->total_lines;
    float div = (float)clock_get_hz(clk_sys) / hfreq / HSYNC_INSTR_LEN;
    sm_config_set_clkdiv(&c, div);

    // Allow PIO to control gpio pin
    pio_gpio_init(pio, HSYNC_PIN);
    pio_sm_set_consecutive_pindirs(pio, sm, HSYNC_PIN, 1, true);
    sm_config_set_set_pins(&c, HSYNC_PIN, 1);

    // Load config and start
    pio_sm_init(pio, sm, offset, &c);
    pio_sm_set_enabled(pio, sm, true);
}

void start_scanline_out(PIO pio, uint sm, uint offset){
    // Prefill the back end with black pixels, allows dumb pixel streaming
    for(int i = 640; i < 800; i++){
        scanline[i] = 0x00;
    }
    
    pio_sm_config c = scanline_out_program_get_default_config(offset);

    // Calculate and set clock divider
    float pixel_clock = timing->total_columns * timing->total_lines * timing->vfreq;
    float div = (float)clock_get_hz(clk_sys) / pixel_clock / SCANLINE_INSTR_LEN;
    sm_config_set_clkdiv(&c, div);

    // Bind pins to state machine
    for(int i = R_MSB_PIN; i < R_MSB_PIN + 8; i++) pio_gpio_init(pio, i);
    sm_config_set_out_pins(&c, R_MSB_PIN, 8);
    pio_sm_set_consecutive_pindirs(pio, sm, R_MSB_PIN, 8, true);
    
    // Set shift register to autopull every 8 bits
    sm_config_set_out_shift(&c, true, true, 8);

    // Setup DMA channel to feed SM FIFO with scanline data
    scanline_dma_channel = dma_claim_unused_channel(true);
    dma_channel_config cfg = dma_channel_get_default_config(scanline_dma_channel);
    channel_config_set_transfer_data_size(&cfg, DMA_SIZE_8);
    channel_config_set_read_increment(&cfg, true);
    channel_config_set_write_increment(&cfg, false);

    // Allow state machine to request DMA start
    channel_config_set_dreq(&cfg, pio_get_dreq(pio, sm, true));

    dma_channel_configure(
        scanline_dma_channel,
        &cfg,
        &pio->txf[sm],   // destination
        scanline,        // source
        800,             // bytes
        false            // Let HSYNC IRQ start DMA stream
    );

    // After full setup, start the state machine
    pio_sm_init(pio, sm, offset, &c);
    pio_sm_set_enabled(pio, sm, true);
}

void __isr hsync_irq_handler(){
    dma_channel_set_read_addr(scanline_dma_channel, scanline, true);
    
    pio_interrupt_clear(pio0, 0);  // clear IRQ 0
    line_count++;

    if (line_count == timing->vsync_start) {
        gpio_put(VSYNC_PIN, 0);   // start VSYNC pulse (active low)
    }
    else if (line_count == timing->vsync_end) {
        gpio_put(VSYNC_PIN, 1);   // end VSYNC pulse
    }
    else if (line_count >= timing->total_lines) {
        line_count = 0;
    }
}

void start_vsync_hook(){
    gpio_init(VSYNC_PIN);
    gpio_set_dir(VSYNC_PIN, GPIO_OUT);
    gpio_put(VSYNC_PIN, 1);
    irq_set_exclusive_handler(PIO0_IRQ_0, hsync_irq_handler);
    irq_set_enabled(PIO0_IRQ_0, true);
    line_count = 0;     // Explicit reset
    pio_set_irq0_source_enabled(pio0, pis_interrupt0, true);
}

// Hardcoding everything for now at native resolution
void render_scanline(){

    uint16_t y = line_count;

    for(uint16_t x = 0; x < 640; x++)
    {
        if(((x >> 3) ^ (y >> 3)) & 1)
            scanline[x] = 0b00000111;
        else
            scanline[x] = 0b11111111;
    }
}