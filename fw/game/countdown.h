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
    Game &game;
public:
    enum State {OFF, RUNNING, FIRED};
    Countdown(Game &_game): game(_game) {}
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
        if(waiting_not_saying && !game.speaker.is_playing()) {
            waiting_not_saying = false;
        }
        /*if(countdown > 1) proj.dimmer(dim = dim * 0.95);
        else proj.dimmer(dim = dim * 0.95);*/
        proj.dimmer(dim = dim * 0.95);
        if(!game.speaker.is_playing() && time_reached(timeout)) {
            if(!game.players.get_steady_count()) game.prepare();
            else {
                game.speaker.saynumber(countdown);
                timeout = make_timeout_time_ms(1000);
                dim = 255;//config.proj_lum;
                countdown--;
            }
        }
        return RUNNING;
    }
    bool pixel_update() {
        if(!running() || waiting_not_saying) return false;
        for(int i = 0; i < config.ring_leds; i++) set_ring_pixel(i, dim, dim, 0);
        for(int i = 0; i < 4; i++) set_spot_pixel(i, dim, dim, 0);
        return true;
    }
    void init(int count) {
        countdown = count;
        timeout = get_absolute_time();
        armed = true;
        waiting_not_saying = true;
    }
};

