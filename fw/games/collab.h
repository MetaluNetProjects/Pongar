// collab game

#pragma once
#include "game.h"
#include "config.h"
#include "countdown.h"
#include "ring_fx.h"
#include "movement.h"

class Collab : public GameMode {
    static const int MAX_LEVEL = 3;
    static const int INIT_PERIOD = 5000;
    static const int MIN_PERIOD = 1000;

    static const int SCORE_MAX = 12;

    int period_ms = INIT_PERIOD;
    float pan, tilt;
    int score;
    int pad_width = 30;
    int flash_count;
    bool end_of_game;
    bool is_winner = false;
    int level = 1;
    Countdown countdown;
    RingFx ringfx;
    MoveCross cross;
    MoveBounce bounce;
    MoveArch arch;
    Movement *move = &cross;

    void set_ring_mode(RingFx::MODE mode, int count) { ringfx.set_mode(mode, count); }
    
    void say_score() {
        say(Words((int)Words::_0 + score));
    }
    void game_over() {
        set_ring_mode(RingFx::LOOSE, 200);
        say(Words::perdu);
        saysilence(1000);
        end_of_game = true;
        is_winner = false;
    }
    void win() {
        set_ring_mode(RingFx::WIN, 200);
        say(Words::gagne);
        saysilence(2000);
        end_of_game = true;
        is_winner = true;
    }
    void init_move(int difficulty) { move->init(pan, tilt, period_ms, difficulty); }

  public:
    virtual ~Collab() {};
    void init() {
        period_ms = INIT_PERIOD;
        score = 0;
        tilt = 0;
        pan = random() % 360;

        move = &cross;
        init_move(0);

        proj.color(0, 0, 0, 255);
        proj.dimmer(128);
        say(Words::partie);
        saysilence(50);
        saynumber(level);
        saysilence(500);
        pad_width = 30;
        set_ring_mode(RingFx::START, 40);
        end_of_game = false;
        is_winner = false;
        countdown.init(3);
    }

    virtual void start() {
        //printf("collab::start\n");
        level = 1;
        init();
    }

    virtual void restart() {
        //printf("collab::restart\n");
        level = level + 1;
        init();
    }

    bool test_touched() {
        int p = (int)pan;
        bool touched = false;
        if(tilt < 0) p = (p + 180) % 360;
        touched = game.players.presence_at(p, pad_width / 2 + 1);
        if(game.get_players_count() == 1) touched |= game.players.presence_at(p + 180, pad_width / 2 + 1);
        if(touched) sfx(SoundCommand::bounce, tilt > 0);
        else {
            sfx(SoundCommand::buzz);
            set_ring_mode(RingFx::FAULT, 20);
        }
        saysilence(300); // waits end of sfx before saying smth
        return touched;
    }

    bool update_score(bool inc) { // return 'end_of_game'
        //printf("update score %d\n", inc);
        if(inc) score++;
        else score--;
        if(score <= 0) { game_over(); return true; }
        else if(score >= SCORE_MAX) { win(); return true; }
        say_score();
        return false;
    }

    virtual void update() {
        if(countdown.update()) return;
        if(end_of_game) {
            if(!game.is_saying()) {
                if(!is_winner) game.prepare();
                else if(level == MAX_LEVEL) game.prepare();
                else game.prepare_restart();
            }
            return;
        }
        proj.dimmer(128);
        if(game.get_players_count() == 1) {}

        if(move->update(pan, tilt)) {
            bool touched = test_touched();
            if(update_score(touched)) return; // end of game

            if(touched) {
                period_ms = period_ms * 0.85;
                if(period_ms < MIN_PERIOD) period_ms = MIN_PERIOD;
            }
            move = &cross;
            if(score > 6 && (random() % 5 == 0)) move = &bounce;
            if(score > 3 && (random() % 5 == 0)) move = &arch;
            init_move(score);
        }

        proj.move(pan, CLIP(tilt, -config.proj_tilt_amp, config.proj_tilt_amp));

        if(move == &bounce) {
            proj.color(255, 0, 0, 0);
            static int x = 0;
            if((x++ % 4) != 0) game.sfx(SoundCommand::tut, 1500, 50);
        } else if(move == &arch) {
            proj.color(0, 0, 255, 0);
            static int x = 0;
            if((x++ % 7) == 0) game.sfx(SoundCommand::tut, 350, 150);
        }
        else proj.color(0, 0, 0, 255);
    }

    virtual void pixels_update() {
        if(countdown.pixel_update()) return;
        if(ringfx.pixel_update()) return;

        int total_leds = MIN(config.total_leds, NUM_PIXELS);

        uint8_t col[3][3] = {{0, 0, 0}, {255, 0, 0}, {255, 255, 255}};
        uint8_t c = 0;
        if(score > 8) {
            flash_count++;
            c = (flash_count / 8) % 2 + 1;
        }
        for(int i = 0; i < total_leds; i++) {
            set_pixel(i, col[0][0], col[0][1], col[0][2]);
        }

        c = 2;
        if(score > 8) {
            flash_count++;
            c = (flash_count / 8) % 2 + 1;
        }
        int r = 255, g = 255, b = 255;
        r = col[c][0];
        g = col[c][1];
        b = col[c][2];
        
        int width = pad_width / 2;
        if(game.get_players_count() == 1) { // double the pad
            for(int i = 0; i < total_leds; i++) {
                int angle = (360 * i) / total_leds - config.leds_angle_offset;
                if(game.players.presence_at(angle, width)) set_pixel(i, r, g, b);
                else if(game.players.presence_at(angle + 180, width)) set_pixel(i, r, g, b);
                else set_pixel(i, 0, 0, 0);
            }
        } else {
            for(int i = 0; i < total_leds; i++) {
                int angle = (360 * i) / total_leds - config.leds_angle_offset;
                if(game.players.presence_at(angle, width)) set_pixel(i, r, g, b);
                else set_pixel(i, 0, 0, 0);
            }
        }
    }
};


