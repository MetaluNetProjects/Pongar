// global game

#include "fraise.h"
#include "game.h"
#include "proj.h"
#include "pixel.h"
#include "config.h"
#include <stdlib.h>

Game game;

#define CLIP(x, min, max) MAX(MIN((x), (max)), (min))

//------------------------------------------------------------
class SimpleMode : public GameMode {
    static const int INIT_PERIOD = 5000;
    static const int MIN_PERIOD = 1200;
    static const int SCORE_MAX = 12;
    static const int INIT_PAN = 30;
    static const int MAX_PAN = 90;
    int period_ms = INIT_PERIOD;
    float pan, tilt;
    float pan_change_amp = 45.0;
    float pan_delta = 0, tilt_delta = 0;
    int new_pan;
    int score;
    void say_score() {
        say(Words((int)Words::_0 + score));
    }
    void game_over() {
        say(Words::perdu);
        saysilence(1000);
        game.prepare();
    }
    void win() {
        say(Words::gagne);
        saysilence(1000);
        game.prepare();
    }

  public:
    virtual ~SimpleMode() {};
    virtual void init() {
        period_ms = INIT_PERIOD;
        score = 0;
        tilt = 0;
        pan = random() % 360;
        pan_delta = 0;
        tilt_delta = (2.0 * config.proj_tilt_amp * game.update_ms) / period_ms;
        pan_change_amp = INIT_PAN;
        proj.color(0, 0, 0, 255);
        proj.dimmer(128);
        say(Words::partie);
        saysilence(100);
        say(Words::_1);
        saysilence(500);
    }

    virtual void update() {
        pan += pan_delta;
        if(pan_delta < 0 && pan < new_pan) pan = new_pan;
        if(pan_delta > 0 && pan > new_pan) pan = new_pan;
        tilt += tilt_delta;
        if((tilt_delta > 0 && tilt >= config.proj_tilt_amp) || (tilt_delta < 0 && tilt <= -config.proj_tilt_amp)) {
            int p = (int)pan;
            bool touched = false;
            if(tilt < 0) p = (p + 180) % 360;
            for(int i = 0; i < game.players.get_count(); i++) {
                if(abs(((players_pos[i] - p + 180 + 360) % 360) - 180) <= (players_separation / 2)) {
                    touched = true;
                    break;
                }
            }
            printf("touched %d\n", touched);
            if(touched) sfx(SoundCommand::bounce, tilt > 0);
            else sfx(SoundCommand::buzz);
            saysilence(300); // waits end of sfx before saying smth

            if(touched) {
                period_ms = period_ms * 0.85;
                if(period_ms < MIN_PERIOD) period_ms = MIN_PERIOD;
                pan_change_amp = CLIP(pan_change_amp + 5, INIT_PAN, MAX_PAN);
            }

            if(touched) score++;
            else score--;
            if(score <= 0) { game_over(); return; }
            else if(score < SCORE_MAX) say_score();
            else { win(); return; }

            if(tilt_delta > 0) tilt_delta = -(2.0 * config.proj_tilt_amp * game.update_ms) / period_ms;
            else tilt_delta = (2.0 * config.proj_tilt_amp * game.update_ms) / period_ms;

            new_pan = pan + ((random() % 2048) - 1024) * (pan_change_amp / 1024);
            new_pan = CLIP(new_pan, 0.0, 360.0);
            pan_delta = 2.0 * ((new_pan - pan) * game.update_ms) / period_ms;
        }
        proj.move(pan, CLIP(tilt, -config.proj_tilt_amp, config.proj_tilt_amp));
    }
} simple_mode;


void Game::init(int audio_pin, int tx_pin) {
    audio.init(audio_pin);
    wavplayer.init(tx_pin);
    prepare();
    game_mode = &simple_mode;
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
    int total_leds = MIN(config.total_leds, NUM_PIXELS);
    if(mode == STOP) return;
    if(mode == PREPARE && players.get_steady_count() == 0) {
        chaser.update();
        return;
    }
    
    for(int i = 0; i < total_leds; i++) {
        set_pixel(i, 30, 0, 0);
    }
    for(int player = 0; player < players_count; player++) {
        int startled = ((players_pos[player] - players_separation / 2 + config.leds_angle_offset + 2) * total_leds) / 360;
        int stopled =  ((players_pos[player] + players_separation / 2 + config.leds_angle_offset - 0) * total_leds) / 360;
        for(int led = startled + 1; led < stopled; led++) set_pixel((led + total_leds) % total_leds, 0, 0, 255);
        set_pixel((startled + total_leds) % total_leds, 255, 255, 0);
        set_pixel((stopled + total_leds) % total_leds, 255, 255, 0);
    }
}

void Game::update() {
    wavplayer.update();
    players.update();
    //pixel_update_players();
    if(!time_reached(update_time)) return;
    update_time = make_timeout_time_ms(update_ms);
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
            if(game_players_count && time_reached(players_ready_timeout) && !is_saying()) {
                players_ready_okcount++;
                if(players_ready_okcount < PLAYERS_READY_SECONDS) {
                    say((Words)((int)Words::_0 + PLAYERS_READY_SECONDS - players_ready_okcount));
                    players_ready_timeout = make_timeout_time_ms(1000);
                    return;
                }
                else start();
            } else return;
            break;
        default: ;
    }
    game_mode->update();
}

void Game::receivebytes(const char* data, uint8_t len) {
    char command = fraise_get_uint8();
    switch(command) {
        case 1: audio.receivebytes(data + 1, len - 1); break;
        case 2: wavplayer.receivebytes(data + 1, len - 1); break;
    }
}

