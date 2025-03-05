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
    //printf("game::prepare\n");
}

void Game::prepare_restart() {
    proj.dimmer(0);
    proj.move(180, 0);
    mode = RESTART;
    wait_saying = true;
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

void Game::change_players_count(int count) {
    game_players_count = count;
    if(game_players_count > 0) {
        speaker.clear();
        speaker.say((Words)((int)Words::_0 + game_players_count));
        speaker.say(Words::joueur);
    }
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
        if(game_players_count != players.get_steady_count()) {
            players_ready_timeout = make_timeout_time_ms(3000);
            change_players_count(players.get_steady_count());
            players_ready_okcount = 0;
            proj.dimmer(0);
            break;
        }
        if(game_players_count && time_reached(players_ready_timeout) && !speaker.is_playing()) start();
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

