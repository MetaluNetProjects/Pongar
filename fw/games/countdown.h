// countdown function

#pragma once
#include "game.h"
class Countdown { // 1 second countdown
  private:
    int countdown;
    absolute_time_t timeout;
    float dim;
    bool armed;
  public:
    bool running() { return ((countdown != 0) || !time_reached(timeout)); }
    bool update() {
        if(!running()) {
            if(armed) {
                game.sfx(SoundCommand::ring, 500);
                game.sfx(SoundCommand::seqplay, 1);
            }
            armed = false;
            return false;
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
        return true;
    }
    bool pixel_update() {
        if(!running()) return false;
        for(int i = 0; i < config.total_leds; i++) set_pixel(i, dim, dim, dim);
        return true;
    }

    void init(int count) {
        countdown = count;
        dim = 255.0;
        timeout = get_absolute_time();
        armed = true;
    }
};

