// glabal game

#include "fraise.h"
#include "game.h"
#include "proj.h"
#include "config.h"
#include <stdlib.h>
#include "sound_command.h"
#include "words.h"

//int Players::get_steady_count(){return steady_count;}
//int Players::get_count(){return players_count;}

void Players::update() {
    if(steady_count == players_count) {
        steady_timeout = at_the_end_of_time;
        pre_steady_count = -1;
        return;
    }
    if(pre_steady_count != players_count) {
        pre_steady_count = players_count;
        steady_timeout = make_timeout_time_ms(STEADY_MS);
        return;
    }
    if(time_reached(steady_timeout)) {
        steady_count = pre_steady_count;
        steady_timeout = at_the_end_of_time;
    }
}

Game game;

#define CLIP(x, min, max) MAX(MIN((x), (max)), (min))

void Game::init(int audio_pin, int tx_pin) {
    audio.init(audio_pin, tx_pin);
    prepare();
}

void Game::prepare() {
    proj_set_light(0);
    mode = PREPARE;
    period_ms = INIT_PERIOD;
    game_players_count = 0;
    score = 0;
}

void Game::start() {
    mode = PLAYING;
    tilt = 0;
    pan_delta = 0;
    tilt_delta = (2.0 * config.proj_tilt_amp * update_ms) / period_ms;
    proj_set_light(128);
    audio.command(SoundCommand::say, (int)Words::partie);
    audio.command(SoundCommand::saypause, 100);
    audio.command(SoundCommand::say, (int)Words::_1);
    audio.command(SoundCommand::saypause, 500);
}

void Game::stop() {
    mode = STOP;
    proj_set_light(0);
}

void Game::change_players_count(int count) {
    game_players_count = count;
    if(game_players_count > 0) {
        audio.command(SoundCommand::sayclear);
        audio.command(SoundCommand::say, (int)Words::_0 + game_players_count);
        audio.command(SoundCommand::say, (int)Words::joueur);
        //audio.command(SoundCommand::saypause, 500);
    }
}

void Game::say_score(){
    audio.command(SoundCommand::say, (int)Words::_0 + score);
}

void Game::game_over() {
    audio.command(SoundCommand::say, (int)Words::perdu);
    prepare();
}

void Game::win(){
    audio.command(SoundCommand::say, (int)Words::gagne);
    prepare();
}

void Game::update() {
    audio.update();
    players.update();
    if(!time_reached(update_time)) return;
    update_time = make_timeout_time_ms(update_ms);
    if(mode == STOP) return;
    if(mode == PREPARE) {
        if(game_players_count != players.get_steady_count()) {
            players_ready_timeout = make_timeout_time_ms(3000);
            change_players_count(players.get_steady_count());
            players_ready_okcount = 0;
            return;
        }
        if(game_players_count && time_reached(players_ready_timeout) && !audio.player_is_playing()) {
            players_ready_okcount++;
            if(players_ready_okcount < PLAYERS_READY_SECONDS) {
                audio.command(SoundCommand::say, (int)Words::_0 + PLAYERS_READY_SECONDS - players_ready_okcount);
                players_ready_timeout = make_timeout_time_ms(1000);
                return;
            }
            else start();
        } else return;
    }
    pan += pan_delta;
    tilt += tilt_delta;
    if((tilt_delta > 0 && tilt >= config.proj_tilt_amp) || (tilt_delta < 0 && tilt <= -config.proj_tilt_amp)) {
        int p = (int)pan;
        bool touched = false;
        if(tilt < 0) p = (p + 180) % 360;
        for(int i = 0; i < players.get_count(); i++) {
            if(abs(((players_pos[i] - p + 180) % 360) - 180) <= (players_separation / 2)) {
                touched = true;
                break;
            }
        }
        printf("touched %d\n", touched);
        if(touched) audio.command(SoundCommand::bounce, tilt > 0);
        else audio.command(SoundCommand::buzz);

        if(touched) period_ms = period_ms * 0.85;
        if(period_ms < MIN_PERIOD) period_ms = MIN_PERIOD;

        if(touched) score++;
        else score--;
        if(score <= 0) game_over();
        else if(score < SCORE_MAX) say_score();
        else win();
        
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

