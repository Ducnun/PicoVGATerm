# Introduction
Signal generation is of course the first step to creating a graphics processor. The video protocol chosen for this project is VGA, as this is a common protocol that is still used in some places, is supported among many displays, supports relatively easy upscaling and is a pretty basic signal. This last part is why the RP2350 can handle the signal generation internally and doesn't rely on external peripherals. 

## Peripherals
The signal generation relies on 2 peripherals available on the RP2350. The first is programmable IO (page 877 of the RP2350 datasheet). There are 3 programmable IO blocks with 4 state machines each. These state machines can perform simple instructions at clock speeds without taking up CPU cycles. This makes them perfect for custom implementations for peripherals that are not present. The second is direct memory access (DMA). This controller can read and write from and to memory without involvement of the CPU. It can also be linked with a PIO state machine to provide the state machine with data automously.

### PIO State machines
#### General idea
The current implementation uses 3 state machines, the HSYNC, VSYNC and RGB state machine. These handle separate parts of the VGA signal. Though VGA does not have a standard resolution or color depth, the most common specification is a resolution of 640x480. This has thus also been implemented in these state machines. Assumptions are made within the state machine code about the exact characteristics of the synchronisation signals and the count of lines and pixels per line.
These 3 state machines are synchronised using interrupts. HSYNC and VSYNC are running at pixel clock speeds, in other words at 25MHz. The RGB state machine is running at 5 times the speed of the HSYNC and VSYNC state machines, so at 125MHz. This state machine is overclocked since it has to perform multiple operations per line. This is because every pixel output requires at least 1 'out' and 1 'jmp' instruction. Additionally, right now pulling from the FIFO is done manually through a third instruction to remove complexity and possible errors from autpull.

#### Synchronisation
HSYNC drives the entire chain. It generates an hsync signal, and towards the end of 1 line signals the VSYNC state machine through an interrupt. The VSYNC state machine runs at the same speed but with less instructions and can thus execute all instructions and then wait until it is signaled again. The VSYNC state machine loops through a visible line for 480 interrupts from HSYNC. During these 480 lines, the output should be active and thus the RGB state machine is started. In lines 490-492 it generates the sync pulse without starting the RGB state machine, then goes back low for another 33 cycles for the full 525 lines. 
The RGB state machine runs 5 times faster than HSYNC and VSYNC and can thus also execute all of its instructions well before it is signaled again. It gets its signal through the interrupt set by HSYNC on the first 480 cycles, representing active lines.
From hsync.pio
```PIOASM
irq 0 [2]
```
This happens towards the end of one loop and triggers the IRQ 0 to signal any state machines listening to it that they can start. In vsync.pio, this signal is used to start the vsync signal and perform some checks:
```PIOASM
activefront: 
    wait 1 irq 0
    irq 1
    jmp x-- activefront
```
This waits for irq 0 to be set, then signals IRQ 1 and loops around to wait on IRQ 0 again, thus relying fully on the hsync state machine. IRQ 1 in turn is used to signal the RGB state machine to start outputting data. 
```PIOASM
wait 1 irq 1 [3]
```
This wait happens every cycle, which is again faster than a single line since RGB data only cover about 80% of an actual line output. The 3 is used as the RGB state machine is clocked faster than hsync and vsync since it needs to perform multiple operations per pixel period.

The orchestration of vsync and rgb output is done through the vsync state machine by using multiple short loops that together create the full loop per frame. Since PIO assembly does not allow variables larger than 31, we pull in a block of data from the FiFo to loop more than 31 times.
```PIOASM
pull block  ; Pull once as jump count for active field is to high for a normal nop

.wrap_target

mov x, osr  ; Copy value from OSR into x, will act as delay ops in the future

activefront: 
    wait 1 irq 0
    irq 1
    jmp x-- activefront ; use prevously pulled x to jump 639 times to start of activefront
    
set y, 9
frontporch:
    wait 1 irq 0
    jmp y-- frontporch
...
...
.wrap
```

### DMA
#### Usage
The main usage of DMA in the signal generation chain is to provide a way for the PIO state machines to pull data out of the SRAM into the state machine's OSR. 
```PIOASM
colorout:
    pull block
    out pins, 8 [3]
    jmp x-- colorout
```
These pulls have to be done by the state machine and are never initiated by the DMA channel. This then allows the state machine to put the pulled byte onto the output pins with a single 'out' instruction. It could be configured to use autopull instead of having the state machine manually pull, but that is currently not needed for performance and reliability and only obfuscates logic.

DMA is setup using 2 channels, one that provides the state machine with the pixel data (aptly called the rgb data channel) and one that resets this first channel (the rgb conf channel). This second channel has one purpose, to write the start of the framebuffer in memory to the start address for the data channel. When the data channel finishes its transfer, it automatically triggers the start of the second channel. This channel then performs it's single write operation into the original buffer and starts the data channel transfer again. 
```C
channel_config_set_chain_to(&c1, rgb_data_channel);

dma_channel_configure(
    rgb_conf_channel, // Channel to be configured
    &c1, // Partial configuration created earlier
    &dma_hw->ch[rgb_data_channel].read_addr, // Write address is start of data dma channel
    &write_ptr, // Read address
    1, // only 1 transfer everytime this dma channel is started
    false // Don't start immediately
);
```
The first line ensures that this DMA channel starts it's transfer when the data channel is done transferring. It then writes the pointer at the start of the framebuffer into the read address of the dma data channel.
