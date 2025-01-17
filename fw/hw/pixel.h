#define WS2812_PIN 2
#define IS_RGBW false
#define NUM_PIXELS 300

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void pixel_setup();
void set_all(bool on);
void pixel_receivebytes(const char* data, uint8_t len);
void pixel_update_players(int players_count, const uint16_t *players_pos, int players_separation);

#ifdef __cplusplus
}
#endif

