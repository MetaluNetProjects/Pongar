#define WS2812_PIN 2
#define IS_RGBW false
#define NUM_PIXELS 300
#include "ws2812.h"
#include <string.h>

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

//extern uint32_t framebuffer[NUM_PIXELS];

extern uint32_t *framebuffer;

void pixel_setup();
void set_all(bool on);
void pixel_receivebytes(const char* data, uint8_t len);
//void pixel_update_players(int players_count, const uint16_t *players_pos, int players_separation);
inline void set_pixel(int n, uint8_t r, uint8_t g, uint8_t b) {
    if(n >= 0 && n < NUM_PIXELS) {
        framebuffer[n] = urgb_u32(r, g, b);
    }
}
inline void clear_pixels() {
    memset(framebuffer, 0, NUM_PIXELS * sizeof(uint32_t));
}

bool pixel_update(/*void (*callback)(void)*/); // return true if frame was sent

#ifdef __cplusplus
}
#endif

