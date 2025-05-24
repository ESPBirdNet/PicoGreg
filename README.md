# Pico Spectrogram Receiver

This project turns a Raspberry Pi Pico into an I²C slave device that receives spectrogram framebuffer data, displays it on an ST7789 display as a heatmap, and blinks the onboard LED to signal status or errors. Offloading spectrogram reception and initial rendering to the Pico frees up memory on an ESP32-S3 for higher-level analysis tasks.

---

## Next Steps

The next goal is to enable audio playback from a microSD card through an I²S speaker, while receiving an array of prediction results from the ESP32-S3 over I²C. This setup will allow the system to play predefined sound samples and collect corresponding model predictions, making it easier to test and evaluate the model’s accuracy, recall, F1 score, and overall performance in a controlled and repeatable way.


---

## Pinout

| Signal              | Pico Pin | ST7789 Pin      |
|---------------------|----------|-----------------|
| SPI SCK             | GP10     | SCK             |
| SPI MOSI            | GP11     | MOSI            |
| SPI CS              | GP12     | CS              |
| SPI DC              | GP13     | DC              |
| SPI RST             | GP14     | RST             |
| SPI BL (Backlight)  | GP15     | BL              |
| I²C SDA             | GP4      | SDA             |
| I²C SCL             | GP5      | SCL             |
| Onboard LED         | GP25     | —               |

---

## Dependencies

- **[pico-sdk](https://github.com/raspberrypi/pico-sdk)**
- **[pico_i2c_slave](https://github.com/vmilea/pico_i2c_slave)** (added as submodule or via CMake FetchContent)

Custom libraries in this repo:

- `display/st7789` — low‑level SPI driver for ST7789 displays  
- `heatmap/heatmap` — palette setup + heatmap rendering routines

---

## Code Overview

1. **`main.c`**  
   - Initializes stdio, onboard LED, display, and I²C slave  
   - Sets up a 128×32 framebuffer + 256-color palette  
   - Calls `display_heatmap_centered()` when a full frame arrives  
   - Blinks LED codes for status (errors, ack, frame received)

2. **`i2c_handler()`**  
   - Runs in interrupt context via `pico_i2c_slave`  
   - Reads either 5-byte commands or bulk framebuffer data  
   - Validates checksum or enters “frame receive” mode  
   - Signals main loop via `blink_code` and `receiving_framebuffer`

3. **`heatmap` module**  
   - `init_palette()` builds an RGB→RGB565 lookup table  
   - `display_heatmap_centered()` scales the 128×32 buffer horizontally and vertically into the 240×320 display window

4. **`st7789` module**  
   - SPI initialization, reset, and command/data helpers  
   - Full-screen fill, pixel‑stream write, window set, color inversion

---

## Purpose & Future Work

The Pico’s primary purpose here is to act as a dedicated I²C slave that safely **receives spectrogram data** and **renders** it, because the ESP32-S3’s limited RAM makes it difficult to simultaneously handle I²C streams, display, and higher‑level DSP/ML analysis. By offloading raw frame buffering and heatmap generation to the Pico, you

- Guarantee smooth data transfer and rendering  
- Free the ESP32-S3 for advanced spectrogram processing (e.g., peak detection, ML inference)  
- Simplify iterative testing of spectrogram display parameters  

Future enhancements might include double buffering, DMA‑accelerated SPI writes, and on‑Pico preprocessing (e.g., log scaling, filtering) before visualization.  
