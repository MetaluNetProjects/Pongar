// countdown function

#pragma once
#include "game.h"
class Countdown { // 1 second countdown
private:
    int countdown;
    absolute_time_t timeout;
    float dim;
    bool armed;
    bool waiting_not_saying;
public:
    enum State {OFF, RUNNING, FIRED};
    bool running() {
        return ((countdown != 0) || !time_reached(timeout));
    }
    State update() {
        if(!running()) {
            if(armed) {
                armed = false;
                return FIRED;
            }
            return OFF;
        }
        if(waiting_not_saying && !game.is_saying()) {
            waiting_not_saying = false;
        }
        if(countdown > 1) proj.dimmer(dim = dim * 0.5);
        else proj.dimmer(dim = dim * 0.8);
        if(!game.is_saying() && time_reached(timeout)) {
            if(!game.players.get_steady_count()) game.prepare();
            else {
                game.saynumber(countdown);
                timeout = make_timeout_time_ms(1000);
                dim = 255.0;
                countdown--;
            }
        }
        return RUNNING;
    }
    bool pixel_update() {
        if(!running() || waiting_not_saying) return false;
        for(int i = 0; i < config.total_leds; i++) set_pixel(i, dim, dim, dim);
        return true;
    }
    void init(int count) {
        countdown = count;
        timeout = get_absolute_time();
        armed = true;
        waiting_not_saying = true;
    }
};

