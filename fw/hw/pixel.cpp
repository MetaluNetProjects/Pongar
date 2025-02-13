#include <stdlib.h>
#include "fraise.h"
#include "pixel.h"
#include "../config.h"

uint32_t framebuffers[2][NUM_PIXELS];
uint32_t *framebuffer = framebuffers[0];
uint32_t all_color0 = 0x00000000;
uint32_t all_color1 = 0xFFFFFFFF;


static absolute_time_t next_time;

void set_all(bool on) {
    uint32_t col = on ? all_color1 : all_color0;
    for(int i = 0; i < NUM_PIXELS - 1; i++) {
        framebuffer[i] = col;
    }
}

void pixel_receivebytes(const char* data, uint8_t len) {
    char command = data[0];
    switch(command) {
    case 10:
        set_all(data[1] != 0);
        break;
    case 20:
        set_pixel((int)data[1], data[2], data[3], data[4]);
        break;
    case 40:
        all_color0 = (int32_t)((data[1] << 24) | (data[2]  << 16) | (data[3] << 8) | data[4]);
        break;
    case 41:
        all_color1 = (int32_t)((data[1] << 24) | (data[2]  << 16) | (data[3] << 8) | data[4]);
        break;
    case 100:
        ws2812_print_status();
        break;
    }
}

void pixel_setup() {
    gpio_set_drive_strength(WS2812_PIN, GPIO_DRIVE_STRENGTH_2MA);
    ws2812_setup(WS2812_PIN, IS_RGBW);
}

bool pixel_update() {
    if(!time_reached(next_time)) return false;
    next_time = make_timeout_time_ms(10);
    memcpy(framebuffers[1], framebuffers[0], sizeof(framebuffers[0]));
    ws2812_dma_transfer(framebuffers[1], NUM_PIXELS);
    return true;
}


