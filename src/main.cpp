#include "pico/stdlib.h"

// I2C communication
#include "hardware/i2c.h"
#include "pico/i2c_slave.h"

// Custom libs
#include "display/st7789.h"
#include "heatmap/heatmap.h"

// I2C config
#define I2C_PORT    i2c0
#define SLAVE_ADDR  0x42

uint8_t framebuffer[FB_W * FB_H];

// General config
volatile uint8_t last_error = 0;

// Spectrogram data
volatile bool receiving_framebuffer = false;
size_t framebuffer_index = 0;

/*
 * I2C 
 */
volatile uint8_t blink_code = 0;

void i2c_handler(i2c_inst_t *i2c, i2c_slave_event_t ev) 
{
     // temp buffer for incoming I²C data
    static uint8_t buf[16];

    if (ev == I2C_SLAVE_RECEIVE) 
    {
        size_t read_len;
        if (receiving_framebuffer) 
        {
            // we’re in "grab framebuffer" mode
            size_t remaining = sizeof(framebuffer) - framebuffer_index;
            if (remaining >= sizeof(buf)) 
            {
                read_len = sizeof(buf);
            } 
            else 
            {
                read_len = remaining;
            }
        } 
        else 
        {
            // normal command is always exactly 5 bytes
            read_len = 5;
        }

        i2c_read_raw_blocking(i2c, buf, read_len);
        size_t count = read_len;

        if (receiving_framebuffer) 
        {
            // copy new chunk into our big framebuffer[]
            for (size_t i = 0; i < count && framebuffer_index < sizeof(framebuffer); ++i) 
            {
                framebuffer[framebuffer_index++] = buf[i];
            }
            // when full, stop receiving and signal with 6 blinks
            if (framebuffer_index >= sizeof(framebuffer)) 
            {
                display_heatmap_centered(framebuffer);
                receiving_framebuffer = false;
                blink_code = 1;
            }
        }
        else 
        {
            // parse a 5-byte command: [B0,B1,B2,B3, checksum]
            uint32_t cmd = ((uint32_t)buf[0] << 24)
                         | ((uint32_t)buf[1] << 16)
                         | ((uint32_t)buf[2] <<  8)
                         | ((uint32_t)buf[3] <<  0);
            uint8_t sum = buf[0] + buf[1] + buf[2] + buf[3];

            if (buf[4] != sum) 
            {
                last_error = 1;    // checksum error
                blink_code = 3;
            }
            else if (cmd == 0xDEADBEEF) 
            {
                last_error = 0;
                blink_code = 1;
            }
            else if (cmd == 0x00FEED00) 
            {
                last_error = 0;
                receiving_framebuffer = true;
                framebuffer_index = 0;
                blink_code = 5;
            }
            else 
            {
                last_error = 2;    // unknown command
                blink_code = 4;
            }
        }
    }
}



/*
 * Main
 */
int main() {
    stdio_init_all();

    // --------- LED (onboard) ----------
    gpio_init(25);
    gpio_set_dir(25, GPIO_OUT);

    // ------ I2C‐SLAVE on GP4/GP5 ------
    st7789_init();
    init_palette();
    
    st7789_fill(0xFFFF);
    sleep_ms(1000);
    st7789_fill(0x0000);

    // ------ I2C‐SLAVE on GP4/GP5 ------
    i2c_init(I2C_PORT, 500 * 1000);     // 500 kHz
    gpio_set_function(4, GPIO_FUNC_I2C);
    gpio_set_function(5, GPIO_FUNC_I2C);
    gpio_pull_up(4);
    gpio_pull_up(5);
    i2c_slave_init(I2C_PORT, SLAVE_ADDR, i2c_handler);

    while (true) {
        if (blink_code > 0) {
            for (int i = 0; i < blink_code; i++) {
                gpio_put(25, 1);
                sleep_ms(50);
                gpio_put(25, 0);
                sleep_ms(150);
            }
            blink_code = 0;
        }
    
        sleep_ms(10);
    }
}
