#include "st7789.h"

// Command codes
#define CMD_SWRESET 0x01
#define CMD_SLPOUT  0x11
#define CMD_COLMOD  0x3A
#define CMD_MADCTL  0x36
#define CMD_INVON   0x21
#define CMD_INVOFF  0x20
#define CMD_DISPON  0x29
#define CMD_CASET   0x2A
#define CMD_RASET   0x2B
#define CMD_RAMWR   0x2C

static int scr_w, scr_h;

// Helper: send one command byte (DC=0)
static void write_command(uint8_t cmd) 
{
    gpio_put(ST7789_DC_PIN, 0);
    gpio_put(ST7789_CS_PIN, 0);
    spi_write_blocking(ST7789_SPI_PORT, &cmd, 1);
    gpio_put(ST7789_CS_PIN, 1);
}

// Helper: send data bytes (DC=1)
static void write_data(const uint8_t *data, size_t len) 
{
    gpio_put(ST7789_DC_PIN, 1);
    gpio_put(ST7789_CS_PIN, 0);
    spi_write_blocking(ST7789_SPI_PORT, data, len);
    gpio_put(ST7789_CS_PIN, 1);
}

// Set rotation / orientation and BGR/RGB order
void st7789_set_rotation(uint8_t madctl_bits) 
{
    write_command(CMD_MADCTL);
    write_data(&madctl_bits, 1);

    // Update width/height based on MV bit (bit 5)
    if (madctl_bits & (1 << 5)) 
    {
        scr_w = ST7789_HEIGHT;
        scr_h = ST7789_WIDTH;
    } 
    else 
    {
        scr_w = ST7789_WIDTH;
        scr_h = ST7789_HEIGHT;
    }
}

// Toggle inversion on/off
void st7789_invert_colors(bool invert) 
{
    write_command(invert ? CMD_INVON : CMD_INVOFF);
}

void st7789_init(void) 
{
    // 1) SPI pins + control pins
    spi_init(ST7789_SPI_PORT, 20 * 1000 * 1000);
    gpio_set_function(ST7789_SCK_PIN,  GPIO_FUNC_SPI);
    gpio_set_function(ST7789_MOSI_PIN, GPIO_FUNC_SPI);

    gpio_init(ST7789_CS_PIN);
    gpio_set_dir(ST7789_CS_PIN, GPIO_OUT);
    gpio_put(ST7789_CS_PIN, 1);

    gpio_init(ST7789_DC_PIN);
    gpio_set_dir(ST7789_DC_PIN, GPIO_OUT);

    gpio_init(ST7789_RST_PIN);
    gpio_set_dir(ST7789_RST_PIN, GPIO_OUT);

    // Backlight on
    gpio_init(ST7789_BL_PIN);
    gpio_set_dir(ST7789_BL_PIN, GPIO_OUT);
    gpio_put(ST7789_BL_PIN, 1);

    // 2) Hardware reset
    gpio_put(ST7789_RST_PIN, 0);
    sleep_ms(10);
    gpio_put(ST7789_RST_PIN, 1);
    sleep_ms(120);

    // 3) Software reset
    write_command(CMD_SWRESET);
    sleep_ms(150);

    // 4) Exit sleep
    write_command(CMD_SLPOUT);
    sleep_ms(150);

    // 5) Turn inversion on by default
    st7789_invert_colors(true);
    sleep_ms(10);

    // 6) Pixel format = 16-bit/pixel
    {
        uint8_t fmt = 0x55; // 16-bit/pixel
        write_command(CMD_COLMOD);
        write_data(&fmt, 1);
        sleep_ms(10);
    }

    // 7) Default rotation: portrait BGR
    {
        uint8_t madctl = (1<<6) /*MY*/ | (1<<5) /*MX*/ | (1<<3) /*RGB*/;
        write_command(CMD_MADCTL);
        write_data(&madctl, 1);       
        st7789_set_rotation(madctl);
    }

    // 8) Display on
    write_command(CMD_DISPON);
    sleep_ms(100);
}

void st7789_set_window(int x, int y, int w, int h) 
{
    // Clamp negative origins
    if (x < 0) { w += x; x = 0; }
    if (y < 0) { h += y; y = 0; }
    if (x + w > scr_w) w = scr_w - x;
    if (y + h > scr_h) h = scr_h - y;

    // Column address set
    write_command(CMD_CASET);
    uint8_t col_args[4] = {
        (uint8_t)(x >> 8), (uint8_t)(x & 0xFF),
        (uint8_t)((x + w - 1) >> 8), (uint8_t)((x + w - 1) & 0xFF)
    };
    write_data(col_args, 4);

    // Row address set
    write_command(CMD_RASET);
    uint8_t row_args[4] = {
        (uint8_t)(y >> 8), (uint8_t)(y & 0xFF),
        (uint8_t)((y + h - 1) >> 8), (uint8_t)((y + h - 1) & 0xFF)
    };
    write_data(row_args, 4);

    // Write to RAM
    write_command(CMD_RAMWR);
}

void st7789_write_pixels(const uint16_t *data, size_t length) 
{
    write_data((const uint8_t *)data, length * 2);
}

void st7789_fill(uint16_t color) 
{
    st7789_set_window(0, 0, scr_w, scr_h);
    static uint16_t line_buffer[ST7789_WIDTH];
    for (int i = 0; i < scr_w; i++) 
    {
        line_buffer[i] = color;
    }
    for (int y = 0; y < scr_h; y++) 
    {
        st7789_write_pixels(line_buffer, scr_w);
    }
}