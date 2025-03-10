// global game

#include "fraise.h"
#include "game.h"
#include "proj.h"
#include "pixel.h"
#include "config.h"
#include <stdlib.h>

#include "collab.h"

Game game;

Collab collab_mode;

void Game::init(int audio_pin, int tx_pin) {
    audio.init(audio_pin);
    speaker.init(tx_pin);
    prepare();
    if(!game_mode) game_mode = &collab_mode;
    chaser.set_mode(0);
}

void Game::prepare() {
    proj.dimmer(0);
    proj.move(180, 0);
    mode = PREPARE;
    wait_saying = true;
    game_players_count = 0;
    noplayer_timeout = players_ready_timeout = players_stable_timeout = at_the_end_of_time;
    last_game_endtime = get_absolute_time();
    next_alpague_time = make_timeout_time_ms((5 + random() % 5) * 1000);
    //printf("game::prepare\n");
}

void Game::prepare_restart() {
    proj.dimmer(0);
    proj.move(180, 0);
    mode = RESTART;
    wait_saying = true;
    noplayer_timeout = players_ready_timeout = players_stable_timeout = at_the_end_of_time;
    //printf("game::prepare_restart\n");
}

void Game::start() {
    if(!game_players_count) prepare();
    else {
        mode = PLAYING;
        game_mode->start();
    }
}

void Game::restart() {
    if(!game_players_count) prepare();
    else {
        mode = PLAYING;
        game_mode->restart();
    }
}

void Game::stop() {
    mode = STOP;
    proj.dimmer(0);
}

void Game::standby() {
    mode = STANDBY;
}

void Game::pixels_update() {
    if(mode == STANDBY) return;
    if(mode == PLAYING) {
        game_mode->pixels_update();
        return;
    }
    int total_leds = MIN(config.total_leds, NUM_PIXELS);
    if(mode == PREPARE && players.get_steady_count() == 0) {
        chaser.update();
        return;
    }

    for(int i = 0; i < total_leds; i++) {
        int angle = (360 * i) / total_leds - config.leds_angle_offset;
        if(game.players.presence_at(angle, 30 / 2)) set_pixel(i, 255, 10, 10);
        else set_pixel(i, 0, 0, 0);
    }
}

bool Game::update() {
    if(!time_reached(update_time)) return false;
    update_time = make_timeout_time_ms(PERIOD_MS);

    speaker.update();
    players.update();

    if(wait_saying) {
        if(speaker.is_playing()) return true;
        else wait_saying = false;
    }

    switch(mode) {
    case STOP:
        break;
    case PREPARE:
        if(!speaker.is_playing() && time_reached(next_alpague_time)) {
            static const int max_rnd_seconds = 5 * 60;
            speaker.say_alpague();
            int rnd_seconds = (get_ms_since_last_game() / 1000) / 20 + 10;
            if(rnd_seconds > max_rnd_seconds) rnd_seconds = max_rnd_seconds;
            next_alpague_time = make_timeout_time_ms((5 + random() % rnd_seconds) * 1000);
        }
        if(players.get_steady_count()) {
            mode = WAIT_STABLE;
            speaker.say(Words::bonjour);
            noplayer_timeout = at_the_end_of_time;
            players_stable_timeout = make_timeout_time_ms((PLAYERS_STABLE_SECONDS + 5) * 1000); // "+ 5" is for accounting 'say(bonjour)' time
        }
        break;
    case WAIT_STABLE:
        if(players.get_steady_count() != 0) noplayer_timeout = make_timeout_time_ms(NO_PLAYER_SECONDS * 1000);
        if(players.get_steady_count() == 0 && time_reached(noplayer_timeout)) {
            mode = PREPARE;
            break;
        }
        if(game_players_count != players.get_steady_count() && !speaker.is_playing()) {
            players_ready_timeout = make_timeout_time_ms(PLAYERS_READY_SECONDS * 1000);
            game_players_count = players.get_steady_count();
            if(time_reached(players_stable_timeout)) {
                speaker.say(Words::attente_joueurs_stables);
                players_stable_timeout = make_timeout_time_ms(PLAYERS_STABLE_SECONDS * 1000);
            }
            proj.dimmer(0);
            break;
        }
        if(game_players_count && time_reached(players_ready_timeout) && !speaker.is_playing()) {
            int max_players = game_mode->get_max_players();
            if(game_players_count <= max_players) start();
            else {
                speaker.say(Words::trop_nombreux);
            }
        }
        break;
        
        break;
    case RESTART:
        if(!speaker.is_playing()) restart();
        break;
    case PLAYING:
        game_mode->update();
        break;
    case STANDBY:
        break;
    }
    return true;
}

void Game::receivebytes(const char* data, uint8_t len) {
    char command = fraise_get_uint8();
    switch(command) {
    case 1:
        audio.receivebytes(data + 1, len - 1);
        break;
    case 2:
        speaker.receivebytes(data + 1, len - 1);
        break;
    }
}

