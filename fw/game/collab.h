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
    static const int TOO_CLOSE_MS = 1000;

    int period_ms = INIT_PERIOD;
    float pan = 0, tilt = 0;
    int score = 0;
    int pad_width = 30;
    int flash_count = 0;
    bool end_of_game = false;
    bool is_winner = false;
    int level = 1;
    int lives = 0;
    int faults = 0;
    Countdown countdown;
    RingFx ringfx;
    Movement *move = nullptr;
    int total_time_ms = 0;
    std::set<int> time_results;
    absolute_time_t too_close_timeout = at_the_end_of_time;
    bool too_close = false;

    void set_move(Movement *newmove) {
        if(move) delete move;
        move = newmove;
    }

    void set_ring_mode(RingFx::MODE mode, int ms) {
        ringfx.set_mode(mode, ms);
    }

    int get_rank(int ms) {
        int rank = 1;
        for(int t: time_results) {
            if(t <= ms) rank++;
            else break;
        }
        return rank;
    }

    void say_win() {
        speaker.say(Words::gagne_partie);
        speaker.say_time(total_time_ms);
        int rank = get_rank(total_time_ms);
        time_results.insert(total_time_ms);
        if(rank <= 5) {
            speaker.say(Words::champion_jour, rank - 1);
        } else {
            speaker.say(Words::classement_jour);
            speaker.saynumber(rank);
        }

        int total_time_cents = total_time_ms / 10;
        int global_rank = game.scorelog.get_rank(total_time_cents);
        if(global_rank <= 5) {
            speaker.say(Words::champion, global_rank - 1);
        } else {
            speaker.say(Words::classement_general);
            speaker.saynumber(global_rank);
        }
        game.scorelog.write(total_time_cents);
    }

    void game_over() {
        lives = lives - 1;
        game.sfx(SoundCommand::seqplay, 0);
        set_ring_mode(RingFx::LOOSE, 800);
        game.sfx(SoundCommand::buzz, 800);
        game.sfx(SoundCommand::sad);
        speaker.silence(2000);
        if(level == 1) {
            speaker.say(Words::perdu_niveau1);
            speaker.silence(10000); // wait 10s before next game
        } else switch(lives) {
            case 0:
                speaker.say(Words::perdu_partie);
                speaker.silence(10000); // wait 10s before next game
                break;
            case 1: speaker.say(Words::perdu_niveau_derniere); break;
            case 2: speaker.say(Words::perdu_niveau); break;
        }
        speaker.silence(1000);
        end_of_game = true;
        is_winner = false;
        score = 0;
        set_move(new MoveEnd(game));
        move->init(pan, tilt, 100, 0);
    }
    void win() {
        game.sfx(SoundCommand::seqplay, 0);
        set_ring_mode(RingFx::WIN, 800);
        game.sfx(SoundCommand::happy);
        speaker.silence(2000);
        switch(level) {
            case 1:
                speaker.say(Words::gagne_niveau_1);
                speaker.silence(300);
                speaker.say(Words::temps_intermediaire);
                speaker.say_time(total_time_ms);
                speaker.silence(300);
                if(faults < 3) {
                    speaker.say(Words::deux_extra_balles);
                    lives = 3;
                } else if(faults < 6) {
                    speaker.say(Words::une_extra_balle);
                    lives = 2;
                } else {
                    speaker.say(Words::pas_extra_balle);
                    lives = 1;
                }
                break;
            case 2:
                speaker.say(Words::gagne_niveau_2);
                speaker.silence(300);
                speaker.say(Words::temps_intermediaire);
                speaker.say_time(total_time_ms);
                break;
            case 3:
                say_win();
                speaker.silence(10000); // wait 10s before next game
                break;
        }
        speaker.silence(1000);
        level = level + 1;
        end_of_game = true;
        is_winner = true;
        score = 0;
        set_move(new MoveEnd(game));
        move->init(pan, tilt, 100, 0);
    }
    void init_move(int difficulty) {
        move->init(pan, tilt, period_ms + (random() % 20), difficulty);
    }
    void set_seq_tempo() {
        game.sfx(SoundCommand::seqms, 100 + period_ms / 8);
    }

    bool mirror_pad() {
        return game.players.get_steady_count() /*game.get_players_count()*/ == 1;
    }

    bool test_touched() {
        int p = (int)pan;
        bool touched = false;
        if(tilt < 0) p = (p + 180) % 360;
        touched = game.players.presence_at(p, pad_width / 2 + 1);
        if(mirror_pad()) touched |= game.players.presence_at(p + 180, pad_width / 2 + 1);
        if(touched) game.sfx(SoundCommand::bounce, tilt > 0);
        else {
            game.sfx(SoundCommand::buzz, 400);
            set_ring_mode(RingFx::FAULT, 400);
            faults++;
        }
        speaker.silence(300); // waits end of sfx before saying smth
        return touched;
    }

    bool update_score(bool inc) { // return 'end_of_game'
        //printf("update score %d\n", inc);
        if(inc) score++;
        else score--;
        if(score < 0) {
            game_over();
            return true;
        }
        else if(score >= SCORE_MAX) {
            win();
            return true;
        }
        speaker.saynumber(score);
        return false;
    }

    void next_move(bool touched) {
        int difficulty = score;
        set_move(new MoveCross(game));
        switch(level) {
        case 1:
            difficulty = score / 2;
            break;
        case 2:
            difficulty = 1 + (score / 2);
            if(score > 5 && (random() % 5 == 0)) set_move(new MoveBounce(game));
            if(score > 6 && (random() % 5 == 0)) set_move(new MoveArch(game));
            break;
        case 3:
            difficulty = 0 + score;
            if(score > 2 && (random() % 3 == 0)) set_move(new MoveBounce(game));
            if(score > 3 && (random() % 4 == 0)) set_move(new MoveArch(game));
            if(score > 4 && (random() % 5 == 0)) set_move(new MoveZigzag(game));
            break;
        }
        if(touched) {
            period_ms = period_ms * 0.85;
            if(period_ms < MIN_PERIOD) period_ms = MIN_PERIOD;
        }
        //set_move(new MoveBounce(game));   // DEBUG!!
        #if 0                               // DEBUG!!
            static int mvcount = 0;
            switch(mvcount) {
                case 0: set_move(new MoveCross(game)); break;
                case 1: set_move(new MoveBounce(game)); break;
                case 2: set_move(new MoveArch(game)); break;
                case 3: set_move(new MoveZigzag(game)); break;
            }
            mvcount = (mvcount + 1) % 4;
        #endif
        init_move(difficulty);
        set_seq_tempo();
    }

public:
    Collab(Game &_game) : GameMode(_game), countdown(_game) {
        set_move(new MoveCross(game));
    }
    virtual ~Collab() {};
    virtual int get_max_players() { return 4; }
    void init() {
        period_ms = INIT_PERIOD - (level - 1) * 750 + (random() % 30);
        score = 0;
        tilt = 0;
        pan = random() % 360;

        set_move(new MoveCross(game));
        init_move(0);

        proj.dimmer(0);
        proj.gobo(level == 1 ? 0 : level == 2 ? 2 : 6);
        proj.color(DMXProj::white);
        proj.move(pan, 0);
        for(int i = 0; i < 4; i++) set_spot_pixel(i, 0, 0, 0);

        if(level == 1) {
            speaker.say(Words::debut_partie);
            speaker.saynumber(game.get_players_count());
            speaker.say(Words::joueur, 0);
        }
        speaker.silence(1500);
        pad_width = 30;
        set_ring_mode(RingFx::START, 1000);
        end_of_game = false;
        is_winner = false;
        countdown.init(3);
        set_seq_tempo();
        game.sfx(SoundCommand::seqnew);
    }

    virtual void start() {
        //printf("collab::start\n");
        level = 1;
        lives = 3;
        faults = 0;
        total_time_ms = 0;
        init();
        //game.sfx(SoundCommand::seqnew);
    }

    virtual void restart() {
        //printf("collab::restart level=%d\n", level);
        init();
    }


    virtual void update() {
        if(game.players.get_too_close() != too_close) {
            too_close = game.players.get_too_close();
            if(! too_close) too_close_timeout = at_the_end_of_time;
            else too_close_timeout = make_timeout_time_ms(TOO_CLOSE_MS);
        }
        auto countstate = countdown.update();
        if(countstate == countdown.RUNNING) return;
        if(countstate == countdown.FIRED) {
            game.sfx(SoundCommand::ring, 500);
            game.sfx(SoundCommand::seqplay, 1);
        }
        if(end_of_game) {
            if(!speaker.is_playing()) {
                if(!is_winner) {
                    if(level == 1 || lives == 0) game.prepare();
                    else game.prepare_restart();
                }
                else if(level == MAX_LEVEL + 1) game.prepare();
                else game.prepare_restart();
            }
            else {
                move->update(pan, tilt);
                proj.move(pan, CLIP(tilt, -config.proj_tilt_amp, config.proj_tilt_amp));
            }
            return;
        }

        proj.dimmer(config.proj_lum);
        //if(game.get_players_count() == 1) {}
        total_time_ms += Game::PERIOD_MS;
        if(move->update(pan, tilt)) {
            bool touched = test_touched();
            if(update_score(touched)) return; // end of game
            next_move(touched);
        }

        if(time_reached(too_close_timeout)) {
            too_close_timeout = make_timeout_time_ms(TOO_CLOSE_MS);
            speaker.clear();
            speaker.say(Words::trop_pres);
            update_score(false);
        }

        proj.move(pan, CLIP(tilt, -config.proj_tilt_amp, config.proj_tilt_amp));
    }

    virtual void pixels_update() {
        if(countdown.pixel_update()) return;
        if(ringfx.pixel_update()) return;

        int ring_leds = MIN(config.ring_leds, NUM_PIXELS);

        uint8_t col[3][3] = {{0, 0, 0}, {255, 0, 0}, {255, 255, 255}};
        uint8_t c = 0;

        for(int i = 0; i < ring_leds; i++) {
            set_ring_pixel(i, col[0][0], col[0][1], col[0][2]);
        }

        c = 2;
        if(score > 9) {
            flash_count++;
            c = (flash_count / 8) % 2 + 1;
        }
        int r = 255, g = 255, b = 255;
        r = col[c][0];
        g = col[c][1];
        b = col[c][2];

        int width = pad_width / 2;
        if(mirror_pad()) { // double the pad
            for(int i = 0; i < ring_leds; i++) {
                int angle = (360 * i) / ring_leds - config.leds_angle_offset;
                if(game.players.presence_at(angle, width)) set_ring_pixel(i, r, g, b);
                else if(game.players.presence_at(angle + 180, width)) set_ring_pixel(i, r, g, b);
                else set_ring_pixel(i, 0, 0, 0);
            }
        } else {
            for(int i = 0; i < ring_leds; i++) {
                int angle = (360 * i) / ring_leds - config.leds_angle_offset;
                if(game.players.presence_at(angle, width)) set_ring_pixel(i, r, g, b);
                else set_ring_pixel(i, 0, 0, 0);
            }
        }

        move->pixel_update();
    }
};

