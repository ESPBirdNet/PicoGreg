#ifndef ST7789_H
#define ST7789_H

#include "hardware/spi.h"
#include "pico/stdlib.h"

#ifdef __cplusplus
extern "C" {
#endif

// Display dimensions
#define ST7789_WIDTH   240
#define ST7789_HEIGHT  320

// Default SPI and control pins
#ifndef ST7789_SPI_PORT
#define ST7789_SPI_PORT   spi1
#endif
#ifndef ST7789_SCK_PIN
#define ST7789_SCK_PIN    10
#endif
#ifndef ST7789_MOSI_PIN
#define ST7789_MOSI_PIN   11
#endif
#ifndef ST7789_CS_PIN
#define ST7789_CS_PIN     12
#endif
#ifndef ST7789_DC_PIN
#define ST7789_DC_PIN     13
#endif
#ifndef ST7789_RST_PIN
#define ST7789_RST_PIN    14
#endif
#ifndef ST7789_BL_PIN
#define ST7789_BL_PIN     15
#endif

// Initialize the display (SPI config + reset sequence)
void st7789_init(void);

// Set drawing window (x, y, width, height)
void st7789_set_window(int x, int y, int w, int h);

// Send a buffer of RGB565 pixels
void st7789_write_pixels(const uint16_t *data, size_t length);

// Fill entire screen with a single RGB565 color
void st7789_fill(uint16_t color);

// Invert display colors
void st7789_invert_colors(bool invert);

#endif // ST7789_H

#ifdef __cplusplus
}
#endif