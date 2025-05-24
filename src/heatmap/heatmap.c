// heatmap.c
#include "heatmap.h"
#include "../display/st7789.h" 

uint16_t palette[256];

void init_palette(void) {
    // Define the four corner colours in 8-bit R,G,B
    const uint8_t stops[4][3] = {
        {  0,   0, 139},  // #00008B (dark blue)
        {  0, 191, 255},  // #00BFFF (light blue)
        {255, 255,   0},  // #FFFF00 (yellow)
        {255,  69,   0}   // #FF4500 (orange)
    };

    // We’ll divide 256 entries into 3 segments of ~85 entries each
    for (int i = 0; i < 256; i++) {
        // Compute which segment we’re in: 0,1,2
        float idx = (i / 255.0f) * 3.0f;
        int   seg = (int)idx;              // 0,1,2
        float t   = idx - seg;             // 0.0 … 1.0 within segment

        // Start/end colours for this segment
        uint8_t *c0 = (uint8_t *)stops[seg];
        uint8_t *c1 = (uint8_t *)stops[seg + 1];

        // Interpolate each 8-bit channel
        uint8_t r8 = c0[0] + (int)((c1[0] - c0[0]) * t + 0.5f);
        uint8_t g8 = c0[1] + (int)((c1[1] - c0[1]) * t + 0.5f);
        uint8_t b8 = c0[2] + (int)((c1[2] - c0[2]) * t + 0.5f);

        // Convert to 5-6-5
        uint16_t r5 = (r8 * 31) / 255;
        uint16_t g6 = (g8 * 63) / 255;
        uint16_t b5 = (b8 * 31) / 255;

        // Pack and store
        palette[i] = (b5 << 11) | (g6 << 5) | r5;
    }
}

void display_heatmap_centered(const uint8_t * framebuffer) {
    const int dst_w = 320;               // Full width of display
    const int dst_h = FB_H * 4;          // Stretch vertically: 32 × 3 = 96
    const int y_offset = (200 - dst_h) / 2 + 20;  // Vertical centering

    const float scale_x = (float)FB_W / dst_w;
    const float scale_y = (float)FB_H / dst_h;

    st7789_set_window(0, y_offset, dst_w, dst_h);

    static uint16_t linebuf[320];        // line buffer for 1 row

    for (int y = 0; y < dst_h; y++) {
        int src_y = (int)(y * scale_y);
        if (src_y >= FB_H) src_y = FB_H - 1;

        uint8_t * src_row = (uint8_t*)(framebuffer + (src_y * FB_W));

        for (int x = 0; x < dst_w; x++) {
            int src_x = (int)(x * scale_x);
            if (src_x >= FB_W) src_x = FB_W - 1;

            uint8_t val = src_row[src_x];
            uint16_t c = palette[val];

            // Convert RGB565 to BGR565 (ST7789 expects BGR)
            c = ((c & 0x1F) << 11) | (c & 0x07E0) | ((c & 0xF800) >> 11);

            linebuf[x] = c;
        }

        st7789_write_pixels(linebuf, dst_w);
    }
}
