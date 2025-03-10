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
};

class MoveCross: public Movement {
protected:
    float pan_change_amp = 45.0;
    float pan_delta = 0, tilt_delta = 0;
    int new_pan;
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
        if(random() % 2) pan_change = -pan_change;
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
};

class MoveBounce: public MoveCross {
private:
    bool tilt_bounce;
public:
    virtual void init(float &pan, float &tilt, int period_ms, int difficulty) {
        MoveCross::init(pan, tilt, period_ms, difficulty);
        tilt_bounce = true;
    }

    virtual bool update(float &pan, float &tilt) {
        if(tilt_bounce && ((tilt_delta > 0 && tilt >= 0) || (tilt_delta < 0 && tilt <= 0))) {
            tilt_delta = -tilt_delta;
            tilt_bounce = false;
        }
        return MoveCross::update(pan, tilt);
    }
};

class MoveArch: public Movement {
protected:
    float index;
    float inc;
    float init_tilt;
    float init_pan;
    int new_pan;
public:
    virtual void init(float &pan, float &tilt, int period_ms, int difficulty) {
        init_tilt = tilt;
        init_pan = pan;
        inc = (1.0 * Game::PERIOD_MS) / period_ms;
        index = 0;

        int pan_change = 180 + ((random() % 60 - 30) * (difficulty + 10)) / 20;
        if(pan < 180) new_pan = pan + pan_change;
        else new_pan = pan - pan_change;
    }

    virtual bool update(float &pan, float &tilt) {
        index += inc;
        tilt = init_tilt * (0.5 + 0.5 * (index * 2.0 - 1.0) * (index * 2.0 - 1.0));
        pan = init_pan + (1.0 - cos(index * 3.14159)) * 0.5 * (new_pan - init_pan);
        bool finished = (index > 1.0);
        return finished;
    }
};

