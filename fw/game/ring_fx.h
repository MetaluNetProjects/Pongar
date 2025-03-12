// ring effects

#pragma once
#include "game.h"
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
        int total_leds = config.total_leds;
        switch(mode) {
        case START: {
            int v = ((count_end - count) * 255) / count_end;
            v = (v * v) / 255;
            for(int i = 0; i < total_leds; i++) {
                set_pixel(i, 0, v, 0);
            }
        }
        break;
        case FAULT: {
            int rot = (count * total_leds) / count_end;
            for(int i = 0; i < total_leds / 4; i++) {
                set_pixel((i + rot) % total_leds, 255, 0, 0);
                set_pixel((i + rot + total_leds / 4) % total_leds, 0, 0, 0);
                set_pixel((i + rot + total_leds / 2) % total_leds, 255, 0, 0);
                set_pixel((i + rot + 3 * total_leds / 4) % total_leds, 0, 0, 0);
            }
        }
        break;
        case WIN: {
            int v = 127 * (1.0 + cos(count * 3.14159 / 40));
            v = (v * v) / 255;
            for(int i = 0; i < total_leds; i++) {
                set_pixel(i, 0, v, 0);
            }
            proj.color(DMXProj::green);
            proj.dimmer(v / 2);
        }
        break;
        case LOOSE: {
            int v = 127 * (1.0 + cos(count * 3.14159 / 40));
            v = (v * v) / 255;
            for(int i = 0; i < total_leds; i++) {
                set_pixel(i, v, 0, 0);
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

