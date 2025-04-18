#define WS2812_PIN 2
#define IS_RGBW false
#define NUM_PIXELS 300
#define PIXEL_PERIOD_MS 10
#include "ws2812.h"
#include <string.h>

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

extern uint32_t *framebuffer;

void pixel_setup();
void set_all(bool on);
void pixel_receivebytes(const char* data, uint8_t len);
inline void set_pixel(int n, uint8_t r, uint8_t g, uint8_t b) {
    if(n >= 0 && n < NUM_PIXELS) {
        framebuffer[n] = urgb_u32(r, g, b);
    }
}
inline void clear_pixels() {
    memset(framebuffer, 0, NUM_PIXELS * sizeof(uint32_t));
}

bool pixel_update(); // return true if frame was sent

#define PIXEL_RING_START (config.spot_leds)
#define PIXEL_SPOT_START 0

#define set_ring_pixel(n, r, g, b) set_pixel((n) + PIXEL_RING_START, r, g, b)
#define set_spot_pixel(n, r, g, b) set_pixel((n) + PIXEL_SPOT_START, r, g, b)

#ifdef __cplusplus
}
#endif

