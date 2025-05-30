// global game

#include "fraise.h"
#include "game.h"
#include "proj.h"
#include "pixel.h"
#include "config.h"
#include <stdlib.h>

#include "collab.h"

void Game::init(int tx_pin) {
    speaker.init(tx_pin);
    prepare();
    if(!game_mode) game_mode = new Collab(*this);
    chaser.set_mode(0);
}

void Game::prepare() {
    proj.dimmer(0);
    proj.move(180, 0);
    state = PREPARE;
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
    state = RESTART;
    wait_saying = true;
    noplayer_timeout = players_ready_timeout = players_stable_timeout = at_the_end_of_time;
    //printf("game::prepare_restart\n");
}

void Game::start() {
    if(!game_players_count) prepare();
    else {
        state = PLAYING;
        game_mode->start();
    }
}

void Game::restart() {
    if(!game_players_count) prepare();
    else {
        state = PLAYING;
        game_mode->restart();
    }
}

void Game::stop() {
    state = STOP;
    proj.dimmer(0);
}

void Game::standby() {
    state = STANDBY;
}

void Game::pixels_update() {
    if(state == STANDBY) return;
    if(state == PLAYING) {
        game_mode->pixels_update();
        return;
    }
    int ring_leds = MIN(config.ring_leds, NUM_PIXELS);
    if(state == PREPARE && players.get_steady_count() == 0) {
        chaser.update();
        return;
    }

    for(int i = 0; i < ring_leds; i++) {
        int angle = (360 * i) / ring_leds - config.leds_angle_offset;
        if(players.presence_at(angle, 30 / 2)) set_ring_pixel(i, 255, 10, 10);
        else set_ring_pixel(i, 0, 0, 0);
    }
    for(int i = 0; i < 4; i++) {
        set_spot_pixel(i, 0, 0, 0);
    }
}

void Game::start_if_not_too_many() {
    int max_players = game_mode->get_max_players();
    if(config.disable_too_many && game_players_count > max_players) {
        game_players_count = max_players;
    }
    if(game_players_count <= max_players) start();
    else {
        speaker.say(Words::trop_nombreux);
        speaker.silence(1000);
    }
}

bool Game::update() {
    if(!time_reached(update_time)) return false;
    update_time = make_timeout_time_ms(PERIOD_MS);

    speaker.update();
    players.update();
    proj.update();

    if(state != STANDBY && players.get_too_close() && (!config.disable_too_close_alarm)) sfx(SoundCommand::tooclose, 20);

    if(wait_saying) {
        if(speaker.is_playing()) return true;
        else wait_saying = false;
    }

    switch(state) {
    case STOP:
        break;
    case PREPARE:
        if(players.get_too_close()) {
            if(!speaker.is_playing()) speaker.say(Words::vous_etes_trop_pres);
            break;
        }
        if(!speaker.is_playing() && time_reached(next_alpague_time)) {
            static const int max_rnd_seconds = 5 * 60;
            speaker.say_alpague();
            int rnd_seconds = (get_ms_since_last_game() / 1000) / 20 + 10;
            if(rnd_seconds > max_rnd_seconds) rnd_seconds = max_rnd_seconds;
            next_alpague_time = make_timeout_time_ms((5 + random() % rnd_seconds) * 1000);
        }
        if(players.get_steady_count()) {
            state = WAIT_STABLE;
            speaker.say(Words::bonjour);
            speaker.silence(1000);
            noplayer_timeout = at_the_end_of_time;
            players_stable_timeout = make_timeout_time_ms(PLAYERS_STABLE_SECONDS * 1000);
            players_say_stable_timeout = players_stable_timeout;
        }
        break;
    case WAIT_STABLE:
        if(players.get_too_close()) {
            if(!speaker.is_playing()) speaker.say(Words::vous_etes_trop_pres);
            break;
        }
        if(players.get_steady_count() != 0) noplayer_timeout = make_timeout_time_ms(NO_PLAYER_SECONDS * 1000);
        if(players.get_steady_count() == 0 && time_reached(noplayer_timeout)) {
            state = PREPARE;
            break;
        }

        if(config.disable_wait_stable) {
            if(!speaker.is_playing()) {
                game_players_count = players.get_steady_count();
                start_if_not_too_many();
            }
            break;
        }

        if(players.count_is_steady()) {
            if(game_players_count != players.get_steady_count()) {
                players_ready_timeout = make_timeout_time_ms(PLAYERS_READY_SECONDS * 1000);
                game_players_count = players.get_steady_count();
            }
        } else {
            if(!speaker.is_playing() && time_reached(players_say_stable_timeout)) {
                speaker.say(Words::attente_joueurs_stables);
                players_stable_timeout = at_the_end_of_time;
                players_say_stable_timeout = make_timeout_time_ms(PLAYERS_STABLE_SECONDS * 1000);
            }
            break;
        }

        if(game_players_count && time_reached(players_ready_timeout) && !speaker.is_playing() && !players.get_too_close()) {
            start_if_not_too_many();
        }
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
    case 2:
        speaker.receivebytes(data + 1, len - 1);
        break;
    }
}

void Game::set_volume(uint8_t vol) {
    volume = vol;
    audio.set_volume(volume);
    speaker.set_volume(volume);
}
