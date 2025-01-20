#include <stdlib.h>
#include "fraise.h"
#include "ws2812.h"
#include "pixel.h"
#include "../config.h"

uint32_t framebuffer[NUM_PIXELS];
//uint32_t framebuffer2[NUM_PIXELS];
//int fb_index;
uint32_t all_color0 = 0x00000000;
uint32_t all_color1 = 0xFFFFFFFF;

void set_pixel(int n, uint8_t r, uint8_t g, uint8_t b) {
	if(n >= 0 && n < NUM_PIXELS) {
		framebuffer[n] = urgb_u32(r, g, b);
	}
}

#if 0
void put_pixel(uint32_t pixel_rgb) {
	if(fb_index < NUM_PIXELS) {
		framebuffer[fb_index++] = pixel_rgb;
	}
}

void framebuffer_send() {
	for(int i = 0; i < NUM_PIXELS; i++)
		ws2812_put_pixel(framebuffer[i]);
}

void pattern_snakes(uint len, uint t) {
    for (uint i = 0; i < len; ++i) {
        uint x = (i + (t >> 1)) % 64;
        if (x < 10)
            put_pixel(urgb_u32(0xff, 0, 0));
        else if (x >= 15 && x < 25)
            put_pixel(urgb_u32(0, 0xff, 0));
        else if (x >= 30 && x < 40)
            put_pixel(urgb_u32(0, 0, 0xff));
        else
            put_pixel(0);
    }
}

void pattern_random(uint len, uint t) {
    if (t % 8)
        return;
    for (uint i = 0; i < len; ++i)
        put_pixel(rand());
}

void pattern_sparkle(uint len, uint t) {
    if (t % 8)
        return;
    for (uint i = 0; i < len; ++i)
        put_pixel(rand() % 16 ? 0 : 0xffffffff);
}

void pattern_greys(uint len, uint t) {
    uint max = 100; // let's not draw too much current!
    t %= max;
    for (uint i = 0; i < len; ++i) {
        put_pixel(t * 0x10101);
        if (++t >= max) t = 0;
    }
}

typedef void (*pattern)(uint len, uint t);
const struct {
    pattern pat;
    const char *name;
} pattern_table[] = {
        {pattern_snakes,  "Snakes!"},
        {pattern_random,  "Random data"},
        {pattern_sparkle, "Sparkles"},
        {pattern_greys,   "Greys"},
};

#endif

//-------------------------------------------//

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
	/*if(pattern_play) {
		if(i++ >= 1000) {
			if(pattern_rnd) {
				pat = rand() % count_of(pattern_table);
				dir = (rand() >> 30) & 1 ? 1 : -1;
			}
			i = 0;
		}
		fb_index = 0;
		pattern_table[pat].pat(NUM_PIXELS, t);
		t += dir;
	}*/
	//framebuffer_send();

	/*rotate_pos += rotate_speed;
	if(rotate_pos <= -(NUM_PIXELS << 16)) rotate_pos += (NUM_PIXELS << 16);
	else if(rotate_pos >= (NUM_PIXELS << 16)) rotate_pos -= (NUM_PIXELS << 16);
	rotate_buffer(rotate_pos >> 16);
	ws2812_dma_transfer(framebuffer2, NUM_PIXELS);*/
	ws2812_dma_transfer(framebuffer, NUM_PIXELS);
	return true;
}

void pixel_setup() {
    gpio_set_drive_strength(WS2812_PIN, GPIO_DRIVE_STRENGTH_2MA);
    ws2812_setup(WS2812_PIN, IS_RGBW);
    add_repeating_timer_ms(20, tick_callback, NULL, &tick_timer);
}

void pixel_update_players(int players_count, const uint16_t *players_pos, int players_separation) {
    int total_leds = config.total_leds;
    if(total_leds > NUM_PIXELS) total_leds = NUM_PIXELS;
    for(int i = 0; i < NUM_PIXELS - 1; i++) {
        framebuffer[i] = all_color0;
    }
    int num = (players_separation * total_leds) / 360;
    for(int player = 0; player < players_count; player++) {
        int start = ((((players_pos[player] - players_separation / 2 + config.leds_angle_offset) * total_leds) / 360) + total_leds) % total_leds;
        for(int led = 0; led <= num; led++) framebuffer[(led + start) % total_leds] = all_color1;
    }
}

