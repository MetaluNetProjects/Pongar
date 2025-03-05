// global game

#pragma once
#include "sound.h"
#include "wavplayer.h"
#include "words.h"

class Speaker: public WavPlayer {
private:
    int say_mode = 2;
public:
    void say(Words w) {
        play(say_mode, (int)w);
    }
    void saynumber(int n) {
        say((Words)((int)Words::_0 + n));
    }
    void saysilence(int ms) {
        silence(ms);
    }
};

