# Hardware
The only custom hardware that is needed for this project is a basic resistor ladder DAC. This turns the 8 digital pin outputs into 3 "analog" signals. These can then be connected to a D-Sub connector for use as a vga signal. The HSYNC, VSYNC and ground pins can be connected directly to the RPI Pico 2. It is recommended to keep connections short to reduce any possible capacitance issues that can occur at high frequencies. For best performance, avoid a breadboard altogether by using a perfboard or PCB (PCB design might be provided in future but currently doesn't exist).

## Schematic
![the hardware schematic](/docs/assets/vga-resistor-dac.png)
The circuit is quite simple and exact pin assignments might change based on custom pin definitions in the platformio.ini file. Keep in mind that the 8 pins for RGB data should be consecutive and use a safe range of pins. HSYNC and VSYNC are more flexible (GPIO 14 and 15).