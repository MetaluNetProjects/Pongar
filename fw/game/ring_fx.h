// ring effects

#pragma once
#include "config.h"

class RingFx {
private:
    float dim;
    int count;
    int count_end;
    int pan;
public:
    enum MODE {OFF, START, GOOD, FAULT, WIN, LOOSE} mode;
    void set_mode(MODE _mode, int ms, int _pan) {
        mode = _mode;
        count = 0;
        count_end = ms / PIXEL_PERIOD_MS;
        pan = _pan;
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
        case GOOD: {
            int pan_led = deg_to_ring(pan); //(pan * ring_leds) / 360;
            float index = count / (float)count_end;
            for(int i = 0; i < ring_leds; i++) {
                int distance = abs(((i - pan_led + (ring_leds * 3) / 2) % ring_leds) - ring_leds / 2);
                int val = 0;
                int alpha = 255.0 * (1.0 - index) / (distance / 20.0 + 1.0);
                if(((int)(distance + (1.0 - index) * 20.0) % 4) != 0) {
                    distance = MAX(0, distance - 2) * 2;
                    val = 255.0 / (distance / (index * 30.0 + 1.0) + 1.0);
                    //alpha = 255.0 * (1.0 - index) / (distance / 20.0 + 1.0);
                }
                blend_ring_pixel(i, 0, val, 0, alpha);
            }
        }
        break;
        case FAULT: {
            int rot = (count * ring_leds) / count_end;
            int pan_led = deg_to_ring(pan); //(pan * ring_leds) / 360;
            float index = count / (float)count_end;
            for(int i = 0; i < ring_leds; i++) {
                int distance = abs(((i - pan_led + (ring_leds * 3) / 2) % ring_leds) - ring_leds / 2);
                distance = MAX(0, distance - 2) * 2;
                int val = 255.0 / (distance / (index * 30.0 + 1.0) + 1.0);
                int alpha = 255.0 * (1.0 - index) / (distance / 20.0 + 1.0);
                blend_ring_pixel(i, val, 0, 0, alpha);
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

