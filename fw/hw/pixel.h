#define WS2812_PIN 2
#define IS_RGBW false
#define NUM_PIXELS 300
#define PIXEL_PERIOD_MS 10
#include "ws2812.h"
#include <string.h>

#pragma once

extern uint32_t *framebuffer;

void pixel_setup();
void set_all(bool on);
void pixel_receivebytes(const char* data, uint8_t len);
inline void set_pixel(int n, uint8_t r, uint8_t g, uint8_t b) {
    if(n >= 0 && n < NUM_PIXELS) {
        framebuffer[n] = urgb_u32(r, g, b);
    }
}

inline void get_pixel(int n, uint8_t &r, uint8_t &g, uint8_t &b) {
    n = MIN(NUM_PIXELS, MAX(0, n));
    u32_urgb(framebuffer[n], &r, &g, &b);
}

inline void clear_pixels() {
    memset(framebuffer, 0, NUM_PIXELS * sizeof(uint32_t));
}

bool pixel_update(); // return true if frame was sent

#define PIXEL_RING_START (config.spot_leds)
#define PIXEL_SPOT_START 0

inline uint8_t blend_color(uint8_t col, uint8_t prev_col, uint8_t alpha) {
    return ((int)col * alpha + (int)prev_col * (255 - alpha)) / 255;
}

inline void blend_pixel(int n, uint8_t r, uint8_t g, uint8_t b, uint8_t alpha) {
    uint8_t prev_r, prev_g, prev_b;
    get_pixel(n, prev_r, prev_g, prev_b);
    set_pixel(n, blend_color(r, prev_r, alpha), blend_color(g, prev_g, alpha), blend_color(b, prev_b, alpha));
}

#define set_ring_pixel(n, r, g, b) set_pixel((n) + PIXEL_RING_START, r, g, b)
#define set_spot_pixel(n, r, g, b) set_pixel((n) + PIXEL_SPOT_START, r, g, b)

#define blend_ring_pixel(n, r, g, b, alpha) blend_pixel((n) + PIXEL_RING_START, r, g, b, alpha)
#define blend_spot_pixel(n, r, g, b, alpha) blend_pixel((n) + PIXEL_SPOT_START, r, g, b, alpha)

#define ring_to_deg(ring_led) ((360 * (ring_led)) / config.ring_leds - config.leds_angle_offset)
#define deg_to_ring(deg) ((((deg) + config.leds_angle_offset) * config.ring_leds) / 360)
