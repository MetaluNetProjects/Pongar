#pragma once

#define NUM_PIXELS 300U
#define PIXEL_PERIOD_MS 10

void set_ring_pixel(int n, uint8_t r, uint8_t g, uint8_t b);
void set_spot_pixel(int n, uint8_t r, uint8_t g, uint8_t b);

void blend_ring_pixel(int n, uint8_t r, uint8_t g, uint8_t b, uint8_t alpha);
void blend_spot_pixel(int n, uint8_t r, uint8_t g, uint8_t b, uint8_t alpha);

#define ring_to_deg(ring_led) ((360 * (ring_led)) / config.ring_leds)
#define deg_to_ring(deg) (((deg) * config.ring_leds) / 360)
