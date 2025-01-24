// global game

#include "fraise.h"
#include "game.h"
#include "proj.h"
#include "pixel.h"
#include "config.h"
#include <stdlib.h>

#include "games/collab.h"

Game game;

Collab collab_mode;

void Game::init(int audio_pin, int tx_pin) {
    audio.init(audio_pin);
    wavplayer.init(tx_pin);
    prepare();
    if(!game_mode) game_mode = &collab_mode;
    chaser.set_mode(0);
}

void Game::prepare() {
    proj.dimmer(0);
    proj.move(180, 0);
    mode = WAIT_SAYING;
    game_players_count = 0;
}

void Game::start() {
    mode = PLAYING;
    game_mode->init();
}

void Game::stop() {
    mode = STOP;
    proj.dimmer(0);
}

void Game::change_players_count(int count) {
    game_players_count = count;
    if(game_players_count > 0) {
        sayclear();
        say((Words)((int)Words::_0 + game_players_count));
        say(Words::joueur);
    }
}

void Game::pixels_update() {
    if(mode == PLAYING) {
        game_mode->pixels_update();
        return;
    }
    int total_leds = MIN(config.total_leds, NUM_PIXELS);
    //if(mode == STOP) return;
    if(mode == PREPARE && players.get_steady_count() == 0) {
        chaser.update();
        return;
    }
    
    for(int i = 0; i < total_leds; i++) {
        set_pixel(i, 30, 0, 0);
    }

    auto pset = game.players.get_set();
    for(int player: pset) {
        if(!game.players.is_visible(player)) continue;
        int width = 50;
        int startled = ((game.players.get_pos(player) - width / 2 + config.leds_angle_offset + 2) * total_leds) / 360;
        int stopled =  ((game.players.get_pos(player) + width / 2 + config.leds_angle_offset - 0) * total_leds) / 360;
        int r, g, b;
        switch(player) {
            case 0: r = 0; g = 255; b = 0; break;
            case 1: r = 0; g = 0; b = 255; break;
            case 2: r = 255; g = 0; b = 255; break;
            default: r = 255; g = 255; b = 0;
        }
        for(int led = startled + 1; led < stopled; led++) set_pixel((led + total_leds) % total_leds, r, g, b);
        set_pixel((startled + total_leds) % total_leds, 0, 0, 0);
        set_pixel((stopled + total_leds) % total_leds, 0, 0, 0);
    }
}

void Game::update() {
    wavplayer.update();
    players.update();
    if(!time_reached(update_time)) return;
    update_time = make_timeout_time_ms(PERIOD_MS);
    switch(mode) {
        case STOP: return; break;
        case WAIT_SAYING:
            if(!is_saying()) mode = PREPARE;
            return;
            break;
        case PREPARE:
            if(game_players_count != players.get_steady_count()) {
                players_ready_timeout = make_timeout_time_ms(3000);
                change_players_count(players.get_steady_count());
                players_ready_okcount = 0;
                return;
            }
            proj.dimmer(dim = dim * 0.5);
            if(game_players_count && time_reached(players_ready_timeout) && !is_saying()) {
                players_ready_okcount++;
                if(players_ready_okcount < PLAYERS_READY_SECONDS) {
                    say((Words)((int)Words::_0 + PLAYERS_READY_SECONDS - players_ready_okcount));
                    players_ready_timeout = make_timeout_time_ms(1000);
                    proj.dimmer(dim = 255.0);
                    return;
                }
                else start();
            } else return;
            break;
        case PLAYING: game_mode->update(); break;
    }
    
}

void Game::receivebytes(const char* data, uint8_t len) {
    char command = fraise_get_uint8();
    switch(command) {
        case 1: audio.receivebytes(data + 1, len - 1); break;
        case 2: wavplayer.receivebytes(data + 1, len - 1); break;
    }
}

