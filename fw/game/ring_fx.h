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
    int led_distance(int led, int from_led, int total_leds) {
        return abs(((led - from_led + (total_leds * 3) / 2) % total_leds) - total_leds / 2);
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
            int pan_led = deg_to_ring(pan);
            float index = count / (float)count_end;
            float index2 = index * index;
            for(int i = 0; i < ring_leds; i++) {
                int distance = led_distance(i, pan_led, ring_leds);
                int val = 500 * (1.0 - index2) / (2.0 * abs(distance - index * 10.0) / (index * 10.0 + 1) + 1.0);
                int alpha = 500 * (1.0 - index2) / (distance / 20.0 + 1.0);
                alpha = MIN(255, MAX(alpha, val));
                val = MIN(255, val);
                blend_ring_pixel(i, 0, val, 0, alpha);
            }
        }
        break;
        case FAULT: {
            int rot = (count * ring_leds) / count_end;
            int pan_led = deg_to_ring(pan);
            float index = count / (float)count_end;
            float index2 = index * index;
            for(int i = 0; i < ring_leds; i++) {
                int distance = led_distance(i, pan_led, ring_leds);
                int val = 500 * (1.0 - index2) / (2.0 * abs(distance - index * 10.0) / (index * 10.0 + 1) + 1.0);
                int val2 = abs(distance - 40.0 * index) < 0.5 ? 255 : 0;
                val = MAX(val2, val);
                int alpha = 500 * (1.0 - index2) / (distance / 20.0 + 1.0);
                alpha = MIN(255, MAX(alpha, val));
                val = MIN(255, val);
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

