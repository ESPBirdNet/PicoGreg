// heatmap.h
#pragma once
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define FB_W 128
#define FB_H  32
#define X_SCALE    2   // 2× width
#define Y_SCALE    4   // 2× height
#define OUT_W   (FB_W*X_SCALE)
#define OUT_H   (FB_H*Y_SCALE)

// init + draw routines
void init_palette(void);
void display_heatmap_centered(const uint8_t * framebuffer);

#ifdef __cplusplus
}
#endif