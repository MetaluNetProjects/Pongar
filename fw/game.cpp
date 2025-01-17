// glabal game

#include "fraise.h"
#include "game.h"
#include "proj.h"
#include "players.h"
#include "config.h"
#include <stdlib.h>

Game game;

#define CLIP(x, min, max) MAX(MIN((x), (max)), (min))

void Game::init(int audio_pin, int tx_pin) {
    audio.init(audio_pin, tx_pin);
}

void Game::start() {
    mode = PLAYING;
    tilt = 0;
    pan_delta = 0;
    tilt_delta = (2.0 * config.proj_tilt_amp * update_ms) / period_ms;
    proj_set_light(128);
}

void Game::stop() {
    mode = STOP;
    proj_set_light(0);
}

void Game::update() {
    audio.update();
    if(!time_reached(update_time)) return;
    update_time = make_timeout_time_ms(update_ms);
    if(mode == STOP) return;
    pan += pan_delta;
    tilt += tilt_delta;
    if((tilt_delta > 0 && tilt >= config.proj_tilt_amp) || (tilt_delta < 0 && tilt <= -config.proj_tilt_amp)) {
        int p = (int)pan; //(360 - (int)pan) % 360;
        bool touched = false;
        if(tilt < 0) p = (p + 180) % 360;
        for(int i = 0; i < players_count; i++) {
            if(abs(((players_pos[i] - p + 180) % 360) - 180) <= (players_separation / 2)) {
                touched = true;
                break;
            }
        }
        printf("touched %d\n", touched);
        if(touched) audio.main_patch.bounce(tilt > 0);
        else audio.main_patch.buzz();
        if(tilt_delta > 0) tilt_delta = -(2.0 * config.proj_tilt_amp * update_ms) / period_ms;
        else tilt_delta = (2.0 * config.proj_tilt_amp * update_ms) / period_ms;
        int new_pan = pan + ((random() % 2048) - 1024) * (pan_change_amp / 1024);
        new_pan = CLIP(new_pan, 0.0, 360.0);
        pan_delta = ((new_pan - pan) * update_ms) / period_ms;
    }
    proj_goto(pan, CLIP(tilt, -config.proj_tilt_amp, config.proj_tilt_amp));
}

void Game::receivebytes(const char* data, uint8_t len) {
    char command = fraise_get_uint8();
    switch(command) {
        case 1: audio.receivebytes(data + 1, len - 1); break;
    }
}

