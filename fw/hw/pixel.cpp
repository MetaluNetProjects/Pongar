#include <stdlib.h>
#include "fraise.h"
#include "pixel.h"
#include "../config.h"

uint32_t framebuffer[NUM_PIXELS];
uint32_t all_color0 = 0x00000000;
uint32_t all_color1 = 0xFFFFFFFF;


static repeating_timer_t tick_timer;

//static int pat;
//static int dir;
//static int t = 0;
//static int i = 1000;
//static bool pattern_play = false;
//static bool pattern_rnd = false;
//static int rotate_pos;
//static int rotate_speed;

/*void rotate_buffer(int steps) {
	for(int i = 0; i < NUM_PIXELS - 1; i++) {
		framebuffer2[i] = framebuffer[(i + steps)%NUM_PIXELS];
	}
}*/

void set_all(bool on) {
    uint32_t col = on ? all_color1 : all_color0;
    for(int i = 0; i < NUM_PIXELS - 1; i++) {
        framebuffer[i] = col;
    }
}

void pixel_receivebytes(const char* data, uint8_t len) {
	char command = data[0];
	switch(command) {
		case 10: set_all(data[1] != 0); break;
		case 20: set_pixel((int)data[1], data[2], data[3], data[4]); break;
		/*case 21:
			pattern_play = data[1] != 0;
			pattern_rnd = data[2] != 0;
			pat = data[3];
			dir = data[4] != 0 ? 1 : -1;
			break;
		case 30: rotate_speed = (int32_t)((data[1] << 24) | (data[2]  << 16) | (data[3] << 8) | data[4]); break;*/
		case 40: all_color0 = (int32_t)((data[1] << 24) | (data[2]  << 16) | (data[3] << 8) | data[4]); break;
		case 41: all_color1 = (int32_t)((data[1] << 24) | (data[2]  << 16) | (data[3] << 8) | data[4]); break;
		case 100: ws2812_print_status(); break;
	}
}

static bool tick_callback(repeating_timer_t *rt)
{
	ws2812_dma_transfer(framebuffer, NUM_PIXELS);
	return true;
}

void pixel_setup() {
    gpio_set_drive_strength(WS2812_PIN, GPIO_DRIVE_STRENGTH_2MA);
    ws2812_setup(WS2812_PIN, IS_RGBW);
    add_repeating_timer_ms(20, tick_callback, NULL, &tick_timer);
}

/*void pixel_update_players(int players_count, const uint16_t *players_pos, int players_separation) {
    int total_leds = MIN(config.total_leds, NUM_PIXELS);
    for(int i = 0; i < NUM_PIXELS - 1; i++) {
        framebuffer[i] = all_color0;
    }
    int num = (players_separation * total_leds) / 360;
    for(int player = 0; player < players_count; player++) {
        int start = ((((players_pos[player] - players_separation / 2 + config.leds_angle_offset) * total_leds) / 360) + total_leds) % total_leds;
        for(int led = 0; led <= num; led++) framebuffer[(led + start) % total_leds] = all_color1;
    }
}*/

