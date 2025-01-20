// global game

#include "fraise.h"
#include "game.h"
#include "proj.h"
#include "config.h"
#include <stdlib.h>
#include "sound_command.h"
#include "words.h"

Game game;

#define CLIP(x, min, max) MAX(MIN((x), (max)), (min))

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

//------------------------------------------------------------
class SimpleMode : public GameMode {
    static const int INIT_PERIOD = 5000;
    static const int MIN_PERIOD = 1200;
    static const int SCORE_MAX = 12;
    int period_ms = INIT_PERIOD;
    float pan, tilt;
    float pan_change_amp = 45.0;
    float pan_delta = 0, tilt_delta = 0;
    int score;
    void say_score() {
        game.audio.command(SoundCommand::say, (int)Words::_0 + score);
    }
    void game_over() {
        game.audio.command(SoundCommand::say, (int)Words::perdu);
        game.prepare();
    }
    void win() {
        game.audio.command(SoundCommand::say, (int)Words::gagne);
        game.prepare();
    }

  public:
    virtual ~SimpleMode() {};
    virtual void init() {
        period_ms = INIT_PERIOD;
        score = 0;
        tilt = 0;
        pan_delta = 0;
        tilt_delta = (2.0 * config.proj_tilt_amp * game.update_ms) / period_ms;
        proj_set_light(128);
        game.audio.command(SoundCommand::say, (int)Words::partie);
        game.audio.command(SoundCommand::saypause, 100);
        game.audio.command(SoundCommand::say, (int)Words::_1);
        game.audio.command(SoundCommand::saypause, 500);
    }
    virtual void update() {
        pan += pan_delta;
        tilt += tilt_delta;
        if((tilt_delta > 0 && tilt >= config.proj_tilt_amp) || (tilt_delta < 0 && tilt <= -config.proj_tilt_amp)) {
            int p = (int)pan;
            bool touched = false;
            if(tilt < 0) p = (p + 180) % 360;
            for(int i = 0; i < game.players.get_count(); i++) {
                if(abs(((players_pos[i] - p + 180) % 360) - 180) <= (players_separation / 2)) {
                    touched = true;
                    break;
                }
            }
            printf("touched %d\n", touched);
            if(touched) game.audio.command(SoundCommand::bounce, tilt > 0);
            else game.audio.command(SoundCommand::buzz);

            if(touched) period_ms = period_ms * 0.85;
            if(period_ms < MIN_PERIOD) period_ms = MIN_PERIOD;

            if(touched) score++;
            else score--;
            if(score <= 0) game_over();
            else if(score < SCORE_MAX) say_score();
            else win();

            if(tilt_delta > 0) tilt_delta = -(2.0 * config.proj_tilt_amp * game.update_ms) / period_ms;
            else tilt_delta = (2.0 * config.proj_tilt_amp * game.update_ms) / period_ms;
            int new_pan = pan + ((random() % 2048) - 1024) * (pan_change_amp / 1024);
            new_pan = CLIP(new_pan, 0.0, 360.0);
            pan_delta = ((new_pan - pan) * game.update_ms) / period_ms;
        }
        proj_goto(pan, CLIP(tilt, -config.proj_tilt_amp, config.proj_tilt_amp));
    }
} simple_mode;


void Game::init(int audio_pin, int tx_pin) {
    audio.init(audio_pin, tx_pin);
    prepare();
    game_mode = &simple_mode;
}

void Game::prepare() {
    proj_set_light(0);
    mode = PREPARE;
    game_players_count = 0;
}

void Game::start() {
    mode = PLAYING;
    game_mode->init();
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
    }
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
    game_mode->update();
}

void Game::receivebytes(const char* data, uint8_t len) {
    char command = fraise_get_uint8();
    switch(command) {
        case 1: audio.receivebytes(data + 1, len - 1); break;
    }
}

