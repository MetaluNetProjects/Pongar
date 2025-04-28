// projector move

#pragma once
#include "game.h"
#include "config.h"
#include <math.h>
class Movement {
protected:
    Game &game;
public:
    Movement(Game &_game): game(_game) {}
    virtual ~Movement() {}
    virtual void init(float &pan, float &tilt, int period_ms, int difficulty) = 0;
    virtual bool update(float &pan, float &tilt) = 0; // return true if finished
    virtual void pixel_update() {}
};

class MoveCross: public Movement {
protected:
    float pan_change_amp = 45.0;
    float pan_delta = 0, tilt_delta = 0;
    int new_pan;
    bool pan_upward;
    static const int INIT_PAN = 15;
    static const int MAX_PAN = 75;
    static const int INC_PAN = 6;
public:
    MoveCross(Game &_game): Movement(_game) {}
    virtual void init(float &pan, float &tilt, int period_ms, int difficulty) {
        tilt_delta = (2.0 * config.proj_tilt_amp * Game::PERIOD_MS) / period_ms;
        if(tilt > 0) tilt_delta = -tilt_delta;

        pan_change_amp = INIT_PAN + INC_PAN * difficulty;
        pan_change_amp = CLIP(pan_change_amp, INIT_PAN, MAX_PAN);
        int pan_change = (0.5 + 0.5 * (random() % 1024) / 1024.0) * pan_change_amp;
        if(tilt > 0) pan_upward = random() % 2;
        if(pan + pan_change > 360) pan_upward = false;
        if(pan - pan_change < 0) pan_upward = true;
        new_pan = pan_upward ? pan + pan_change : pan - pan_change;
        new_pan = CLIP(new_pan, 0.0, 360.0);
        pan_delta = 2.0 * ((new_pan - pan) * Game::PERIOD_MS) / period_ms;
    }

    virtual bool update(float &pan, float &tilt) {
        pan += pan_delta;
        if(pan_delta < 0 && pan < new_pan) pan = new_pan;
        if(pan_delta > 0 && pan > new_pan) pan = new_pan;
        tilt += tilt_delta;
        bool finished = ((tilt_delta > 0 && tilt >= config.proj_tilt_amp) || (tilt_delta < 0 && tilt <= -config.proj_tilt_amp));

        proj.color(DMXProj::white);

        return finished;
    }

    virtual void pixel_update() {
        for(int i = 0; i < 4; i++) {
            set_spot_pixel(i, 0, 0, 0);
        }
    }
};

class MoveBounce: public MoveCross {
private:
    bool tilt_bounce;
    int ms;
    int sfxcount;
public:
    MoveBounce(Game &_game): MoveCross(_game) {}

    virtual void init(float &pan, float &tilt, int period_ms, int difficulty) {
        MoveCross::init(pan, tilt, period_ms * 1.5, difficulty);
        tilt_bounce = true;
        ms = sfxcount = 0;
    }

    virtual bool update(float &pan, float &tilt) {
        if(tilt_bounce && ((tilt_delta > 0 && tilt >= 0) || (tilt_delta < 0 && tilt <= 0))) {
            tilt_delta = -tilt_delta;
            tilt_bounce = false;
        }
        bool finished = MoveCross::update(pan, tilt);

        proj.color(DMXProj::red);
        if(sfxcount != ms / 75) {
            sfxcount = ms / 75;
            if((sfxcount % 5) != 0) game.sfx(SoundCommand::tut, 1500, 50);
        }
        ms += Game::PERIOD_MS;

        return finished;
    }

    virtual void pixel_update() {
        for(int i = 0; i < 4; i++) {
            set_spot_pixel(i, abs(sin((ms * 3.0 / 1000.0 + i / 4.0) * 3.14)) * 150.0, 0, 0);
        }
    }
};

class MoveArch: public Movement {
protected:
    float index;
    float inc;
    float init_tilt;
    float init_pan;
    int new_pan;
    int ms;
    int sfxcount;
public:
    MoveArch(Game &_game): Movement(_game) {}

    virtual void init(float &pan, float &tilt, int period_ms, int difficulty) {
        period_ms = (period_ms * 1.2);
        init_tilt = tilt;
        init_pan = pan;
        inc = (1.0 * Game::PERIOD_MS) / period_ms;
        index = 0;

        int pan_change = 180 + ((random() % 120 - 60) * (difficulty + 10)) / 20;
        if(pan + pan_change > 360 || pan + pan_change < 0) new_pan = pan - pan_change;
        else new_pan = pan + pan_change;
        new_pan = CLIP(new_pan, 0.0, 360.0);
        ms = 0;
    }

    virtual bool update(float &pan, float &tilt) {
        index += inc;
        tilt = init_tilt * (0.3 + 0.7 * (index * 2.0 - 1.0) * (index * 2.0 - 1.0));
        pan = init_pan + (1.0 - cos(index * 3.14159)) * 0.5 * (new_pan - init_pan);
        bool finished = (index > 1.0);

        proj.color(DMXProj::blue);
        if(sfxcount != ms / 40) {
            sfxcount = ms / 40;
            if((sfxcount % 7) == 0) game.sfx(SoundCommand::tut, 350, 150);
        }
        ms += Game::PERIOD_MS;

        return finished;
    }

    virtual void pixel_update() {
        for(int i = 0; i < 4; i++) {
            set_spot_pixel(i, 0, 0, abs(sin((ms * 2.0 / 1000.0 + i / 4.0) * 3.14)) * 150.0);
        }
    }
};

class MoveZigzag: public MoveCross {
private:
    float index;
    float inc;
    float old_pan;
    int ms;
public:
    MoveZigzag(Game &_game): MoveCross(_game) {}

    virtual void init(float &pan, float &tilt, int period_ms, int difficulty) {
        period_ms = (period_ms * 1.8);
        MoveCross::init(pan, tilt, period_ms, difficulty);
        inc = (1.0 * Game::PERIOD_MS) / period_ms;
        index = 0;
        old_pan = pan;
        ms = 0;
    }

    virtual bool update(float &pan, float &tilt) {
        pan = old_pan;
        bool finished = MoveCross::update(pan, tilt);
        old_pan = pan;
        index += inc;
        pan += sin(index * 3.14158 * 3) * sin(index * 3.14158) * 20;
        pan = CLIP(pan, 0.0, 360.0);

        proj.color(DMXProj::green);
        uint8_t val = (((10 * ms) / 1000) % 2) * config.proj_lum;
        proj.dimmer(val);
        game.sfx(SoundCommand::tut, 700 + 250 * sinf((ms * 8.0 * 3.14) / 1000.0), 80);
        ms += Game::PERIOD_MS;

        return finished;
    }

    virtual void pixel_update() {
        uint8_t val = (((13 * ms) / 1000) % 2) * 200;
        for(int i = 0; i < 4; i++) {
            //set_spot_pixel(i, 0, abs(sin((ms * 4.0 / 1000.0 + i / 4.0) * 3.14)) * 150.0, 0);
            set_spot_pixel(i, 0, val, 0);
        }
    }
};

class MoveEndWin: public Movement {
protected:
    int ms;
    float panf, tiltf;
public:
    MoveEndWin(Game &_game): Movement(_game) {}

    virtual void init(float &pan, float &tilt, int period_ms, int difficulty) {
        panf = pan;
        tiltf = tilt;
        ms = 0;
    }

    virtual bool update(float &pan, float &tilt) {
        panf += (180.0 - panf) * 0.01;
        tiltf = tiltf * 0.99;
        pan = panf;
        tilt = tiltf;
        int c = ((ms * 2) / 1000) % 6;
        proj.color((DMXProj::Color)c);
        proj.dimmer((0.5 + 0.5 * sin((ms * 1.5 / 1000.0) * 6.28)) * 255 / (1 + ms / 2000));
        ms += Game::PERIOD_MS;
        return false;
    }

    inline int sinval(float offset) {
        float val = 10.0 * sin((ms * 2.0 / 1000.0 + offset) * 6.28);
        val = CLIP(val, -1, 1);
        val = 255.0 * 0.5 * (val + 1.0);
        return val;
    }

    virtual void pixel_update() {
        int val[4] = { sinval(0.0), sinval(0.25), sinval(0.50), sinval(0.75) };
        for(int i = 0; i < 4; i++) {
            set_spot_pixel(i, val[(i + 0) % 4], val[((i * 3) / 2 + 1) % 4], val[((i * 5) / 2 + 2) % 4]);
        }
    }
};

class MoveEndLoose: public MoveEndWin {
public:
    MoveEndLoose(Game &_game): MoveEndWin(_game) {}

    virtual bool update(float &pan, float &tilt) {
        MoveEndWin::update(pan, tilt);
        proj.color(DMXProj::red);
        return false;
    }

    virtual void pixel_update() {
        int val[4] = { sinval(0.0), sinval(0.25), sinval(0.50), sinval(0.75) };
        for(int i = 0; i < 4; i++) {
            set_spot_pixel(i, val[i], 0, 0);
        }
    }
};

