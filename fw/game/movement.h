// projector move

#pragma once
#include "game.h"
#include "config.h"
#include <math.h>
class Movement {
private:
public:
    virtual void init(float &pan, float &tilt, int period_ms, int difficulty) = 0;
    virtual bool update(float &pan, float &tilt) = 0; // return true if finished
    virtual void fx_update() = 0;
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
    virtual void init(float &pan, float &tilt, int period_ms, int difficulty) {
        tilt_delta = (2.0 * config.proj_tilt_amp * Game::PERIOD_MS) / period_ms;
        if(tilt > 0) tilt_delta = -tilt_delta;

        pan_change_amp = INIT_PAN + INC_PAN * difficulty;
        pan_change_amp = CLIP(pan_change_amp, INIT_PAN, MAX_PAN);
        int pan_change = (0.5 + 0.5 * (random() % 1024) / 1024.0) * pan_change_amp;
        if(tilt > 0) pan_upward = random() % 2;
        if(!pan_upward) pan_change = -pan_change;
        new_pan = pan + pan_change;
        new_pan = CLIP(new_pan, 0.0, 360.0);
        pan_delta = 2.0 * ((new_pan - pan) * Game::PERIOD_MS) / period_ms;
    }

    virtual bool update(float &pan, float &tilt) {
        pan += pan_delta;
        if(pan_delta < 0 && pan < new_pan) pan = new_pan;
        if(pan_delta > 0 && pan > new_pan) pan = new_pan;
        tilt += tilt_delta;
        bool finished = ((tilt_delta > 0 && tilt >= config.proj_tilt_amp) || (tilt_delta < 0 && tilt <= -config.proj_tilt_amp));
        return finished;
    }

    virtual void fx_update() {
        proj.color(DMXProj::white);
    }
};

class MoveBounce: public MoveCross {
private:
    bool tilt_bounce;
    int ms;
    int sfxcount;
public:
    virtual void init(float &pan, float &tilt, int period_ms, int difficulty) {
        MoveCross::init(pan, tilt, period_ms, difficulty);
        tilt_bounce = true;
        ms = sfxcount = 0;
    }

    virtual bool update(float &pan, float &tilt) {
        if(tilt_bounce && ((tilt_delta > 0 && tilt >= 0) || (tilt_delta < 0 && tilt <= 0))) {
            tilt_delta = -tilt_delta;
            tilt_bounce = false;
        }
        return MoveCross::update(pan, tilt);
    }

    virtual void fx_update() {
        proj.color(DMXProj::red);
        if(sfxcount != ms / 75) {
            sfxcount = ms / 75;
            if((sfxcount % 5) != 0) game.sfx(SoundCommand::tut, 1500, 50);
        }
        ms += Game::PERIOD_MS;
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
    virtual void init(float &pan, float &tilt, int period_ms, int difficulty) {
        init_tilt = tilt;
        init_pan = pan;
        inc = (1.0 * Game::PERIOD_MS) / period_ms;
        index = 0;

        int pan_change = 180 + ((random() % 120 - 60) * (difficulty + 10)) / 20;
        if(pan < 180) new_pan = pan + pan_change;
        else new_pan = pan - pan_change;
        ms = 0;
    }

    virtual bool update(float &pan, float &tilt) {
        index += inc;
        tilt = init_tilt * (0.3 + 0.7 * (index * 2.0 - 1.0) * (index * 2.0 - 1.0));
        pan = init_pan + (1.0 - cos(index * 3.14159)) * 0.5 * (new_pan - init_pan);
        bool finished = (index > 1.0);
        return finished;
    }

    virtual void fx_update() {
        proj.color(DMXProj::blue);
        if(sfxcount != ms / 40) {
            sfxcount = ms / 40;
            if((sfxcount % 7) == 0) game.sfx(SoundCommand::tut, 350, 150);
        }
        ms += Game::PERIOD_MS;
    }
};

class MoveZigzag: public MoveCross {
private:
    float index;
    float inc;
    float old_pan;
    int ms;
public:
    virtual void init(float &pan, float &tilt, int period_ms, int difficulty) {
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
        pan += sin(index * 3.14158 * 6) * sin(index * 3.14158) * 20;
        pan = CLIP(pan, 0.0, 360.0);
        return finished;
    }

    virtual void fx_update() {
        proj.color(DMXProj::green);
        game.sfx(SoundCommand::tut, 700 + 250 * sinf((ms * 8.0 * 3.14) / 1000.0), 80);
        ms += Game::PERIOD_MS;
    }
};


