// ring effects

#pragma once
#include "config.h"

class RingFx {
private:
    float dim;
    int count;
    int count_end;
public:
    enum MODE {OFF, START, FAULT, WIN, LOOSE} mode;
    void set_mode(MODE _mode, int ms) {
        mode = _mode;
        count = 0;
        count_end = ms / PIXEL_PERIOD_MS;
    }
    bool pixel_update() {
        if(mode == OFF) return false;
        int ring_leds = config.ring_leds;
        switch(mode) {
        case START: {
            int v = ((count_end - count) * 255) / count_end;
            v = (v * v) / 255;
            for(int i = 0; i < ring_leds; i++) {
                set_ring_pixel(i, 0, v, 0);
            }
            for(int i = 0; i < 4; i++) {
                set_spot_pixel(i, 0, 255 - (255 * count) / count_end, 0);
            }
        }
        break;
        case FAULT: {
            int rot = (count * ring_leds) / count_end;
            for(int i = 0; i < ring_leds / 4; i++) {
                set_ring_pixel((i + rot) % ring_leds, 255, 0, 0);
                set_ring_pixel((i + rot + ring_leds / 4) % ring_leds, 0, 0, 0);
                set_ring_pixel((i + rot + ring_leds / 2) % ring_leds, 255, 0, 0);
                set_ring_pixel((i + rot + 3 * ring_leds / 4) % ring_leds, 0, 0, 0);
            }
            for(int i = 0; i < 4; i++) {
                set_spot_pixel(i, ((i + rot / 5) % 4 != 0) * (255 - (255 * count) / count_end), 0, 0);
            }
        }
        break;
        case WIN: {
            int v = 127 * (1.0 + cos(count * 3.14159 / 40));
            v = (v * v) / 255;
            for(int i = 0; i < ring_leds; i++) {
                set_ring_pixel(i, 0, v, 0);
            }
            proj.color(DMXProj::green);
            proj.dimmer(v / 2);
            for(int i = 0; i < 4; i++) {
                set_spot_pixel(i, v * 255, v * 255, 0);
            }
        }
        break;
        case LOOSE: {
            int v = 127 * (1.0 + cos(count * 3.14159 / 40));
            v = (v * v) / 255;
            for(int i = 0; i < ring_leds; i++) {
                set_ring_pixel(i, v, 0, 0);
            }
            proj.color(DMXProj::red);
            proj.dimmer(v / 2);
        }
        break;
        default:
            ;
        }
        if(count++ >= count_end) {
            mode = OFF;
        }
        return true;
    }
};

