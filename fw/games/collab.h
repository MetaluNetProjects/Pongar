// collab game

#pragma once
#include "game.h"

class Collab : public GameMode {
    static const int INIT_PERIOD = 5000;
    static const int MIN_PERIOD = 1000;

    static const int SCORE_MAX = 12;
    static const int INIT_PAN = 15;
    static const int MAX_PAN = 60;
    static const int INC_PAN = (MAX_PAN - INIT_PAN ) / (float)SCORE_MAX;
    int period_ms = INIT_PERIOD;
    float pan, tilt;
    float pan_change_amp = 45.0;
    float pan_delta = 0, tilt_delta = 0;
    int new_pan;
    int score;
    int pad_width = 30;
    int tilt_bounce = 1000;
    int flash_count;
    bool end_of_game;
    
    enum RING_MODE {NORMAL, START, FAULT, WIN, LOOSE} ring_mode;
    int ring_count;
    int ring_end;
    void set_ring_mode(RING_MODE mode, int count) { ring_mode = mode; ring_count = 0; ring_end = count; }
    
    void say_score() {
        say(Words((int)Words::_0 + score));
    }
    void game_over() {
        set_ring_mode(LOOSE, 200);
        say(Words::perdu);
        saysilence(1000);
        end_of_game = true;
        //game.prepare();
    }
    void win() {
        set_ring_mode(WIN, 200);
        say(Words::gagne);
        saysilence(2000);
        end_of_game = true;
        //game.prepare();
    }

  public:
    virtual ~Collab() {};
    virtual void init() {
        period_ms = INIT_PERIOD;
        score = 0;
        tilt = 0;
        pan = random() % 360;
        pan_delta = 0;
        tilt_delta = (2.0 * config.proj_tilt_amp * Game::PERIOD_MS) / period_ms;
        pan_change_amp = INIT_PAN;
        proj.color(0, 0, 0, 255);
        proj.dimmer(128);
        say(Words::partie);
        saysilence(100);
        say(Words::_1);
        saysilence(500);
        pad_width = 30;
        tilt_bounce = 1000;
        set_ring_mode(START, 40);
        end_of_game = false;
    }

    bool test_touched() {
        int p = (int)pan;
        bool touched = false;
        if(tilt < 0) p = (p + 180) % 360;
        //for(int i = 0; i < game.players.get_count(); i++) {
        /*for(int i: game.players.get_set()) {
            if(game.players.is_visible(i) && abs(((game.players.get_pos(i).angle - p + 180 + 360) % 360) - 180) <= (pad_width / 2)) {
                touched = true;
                break;
            }
        }*/
        touched = game.players.presence_at(p, pad_width / 2 + 1);
        if(game.get_players_count() == 1) touched |= game.players.presence_at(p + 180, pad_width / 2 + 1);
        //printf("touched %d\n", touched);
        if(touched) sfx(SoundCommand::bounce, tilt > 0);
        else {
            sfx(SoundCommand::buzz);
            set_ring_mode(FAULT, 20);
        }
        saysilence(300); // waits end of sfx before saying smth
        return touched;
    }

    bool update_score(bool inc) { // return 'end_of_game'
        if(inc) score++;
        else score--;
        if(score <= 0) { game_over(); return true; }
        else if(score >= SCORE_MAX) { win(); return true; }
        say_score();
        return false;
    }

    virtual void update() {
        if(end_of_game) {
            if(!game.is_saying()) game.prepare();
            return;
        }
        if(game.get_players_count() == 1) {
        }
        pan += pan_delta;
        if(pan_delta < 0 && pan < new_pan) pan = new_pan;
        if(pan_delta > 0 && pan > new_pan) pan = new_pan;
        if((tilt_bounce != 1000) && ((tilt_delta > 0 && tilt >= tilt_bounce) || (tilt_delta < 0 && tilt <= tilt_bounce))) {
            tilt_delta = -tilt_delta;
            tilt_bounce = 1000;
        }
        tilt += tilt_delta;
        if((tilt_delta > 0 && tilt >= config.proj_tilt_amp) || (tilt_delta < 0 && tilt <= -config.proj_tilt_amp)) {
            bool touched = test_touched();
            if(update_score(touched)) return; // end of game

            if(touched) {
                period_ms = period_ms * 0.85;
                if(period_ms < MIN_PERIOD) period_ms = MIN_PERIOD;
                pan_change_amp = CLIP(pan_change_amp + INC_PAN, INIT_PAN, MAX_PAN);
            }

            if(tilt_delta > 0) tilt_delta = -(2.0 * config.proj_tilt_amp * Game::PERIOD_MS) / period_ms;
            else tilt_delta = (2.0 * config.proj_tilt_amp * Game::PERIOD_MS) / period_ms;

            int pan_change = (0.5 + 0.5 * (random() % 1024) / 1024.0) * pan_change_amp;
            if(random()%2) pan_change = -pan_change;
            new_pan = pan + pan_change;
            new_pan = CLIP(new_pan, 0.0, 360.0);
            pan_delta = 2.0 * ((new_pan - pan) * Game::PERIOD_MS) / period_ms;
            tilt_bounce = 1000;
            if(score > 6 && (random() % 5 == 0)) tilt_bounce = 0;
            //if(score > 8 && (random() % 6 == 0)) tilt_bounce = tilt_delta > 0 ? -10 : 10;
            if(tilt_bounce != 1000) proj.color(255, 0, 0, 0);
            else proj.color(0, 0, 0, 255);
        }
        if(tilt_bounce != 1000) {
            static int x = 0;
            if(x++ % 4) game.sfx(SoundCommand::tut, 1500, 50);
        }
        proj.move(pan, CLIP(tilt, -config.proj_tilt_amp, config.proj_tilt_amp));
    }

    void ring_mode_paint(int total_leds) {
        switch(ring_mode) {
            case NORMAL: break;
            case START: {
                    int v = ((ring_end - ring_count) * 255) / ring_end;
                    v = (v * v) / 255;
                    for(int i = 0; i < total_leds; i++) {
                        set_pixel(i, 0, v, 0);
                    }
                }
                break;
            case FAULT: {
                    int rot = (ring_count * total_leds) / ring_end;
                    for(int i = 0; i < total_leds / 4; i++) {
                        set_pixel((i + rot) % total_leds, 255, 0, 0);
                        set_pixel((i + rot + total_leds / 4) % total_leds, 0, 0, 0);
                        set_pixel((i + rot + total_leds / 2) % total_leds, 255, 0, 0);
                        set_pixel((i + rot + 3 * total_leds / 4) % total_leds, 0, 0, 0);
                    }
                }
                break;
            case WIN: {
                    int v = 127 * (1.0 + cos(ring_count * 3.14159 / 40));
                    v = (v * v) / 255;
                    for(int i = 0; i < total_leds; i++) {
                        set_pixel(i, 0, v, 0);
                    }
                }
                break;
            case LOOSE: {
                    int v = 127 * (1.0 + cos(ring_count * 3.14159 / 40));
                    v = (v * v) / 255;
                    for(int i = 0; i < total_leds; i++) {
                        set_pixel(i, v, 0, 0);
                    }
                }
                break;
            default: ;
        }
        if(ring_count++ == ring_end) {
            ring_mode = NORMAL;
        }
    }

    virtual void pixels_update() {
        int total_leds = MIN(config.total_leds, NUM_PIXELS);

        if(ring_mode != NORMAL || end_of_game) {
            ring_mode_paint(total_leds);
            return;
        }
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


