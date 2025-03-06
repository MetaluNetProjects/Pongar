// global game

#pragma once
#include "sound.h"
#include "wavplayer.h"
#include "words.h"

class Speaker: public WavPlayer {
private:
    int mode = 2;
public:
    static const bool FEMALE = true;
    static const bool MALE = false;

    void set_mode(int m) {
        mode = m;
    }

    void say(Words w, int offset = 0) {
        play(mode, (int)w + offset);
    }

    void saynumber(int n, bool gender = MALE) {
        bool say_zero = true;
        if(n >= 1000) {
            int m = n / 1000;
            n %= 1000;
            m = m % 1000;
            if(m > 1) saynumber(m, gender);
            say(Words::_1000);
            say_zero = false;
        }
        if(n >= 100) {
            int m = n / 100;
            n %= 100;
            if(m > 1) saynumber(m, gender);
            say(Words::_100);
            say_zero = false;
        }
        if(n >= 21) {
            int m = n / 10;
            if(m == 7) m = 6;
            else if(m == 9) m = 8;
            n -= m * 10;
            say((Words)((int)Words::_20 + m - 2));
            if((n == 1 || n == 11) && m != 8) say(Words::et);
            if(n != 0) saynumber(n, gender); //say(Words::_0, n);
        }
        else {
            if(n == 0 && !say_zero) return;
            if(n == 1 && gender == FEMALE) say(Words::une);
            else say(Words::_0, n);
        }
    }

    void say_hundredths(float f) {
        int n = (int)f;
        int m = (f - (float)n) * 100;
        saynumber(n);
        if(m) {
            say(Words::et);
            saynumber(m);
            say(Words::centieme);
        }
    }

    void say_time(int ms) {
        int mins = (ms / 1000) / 60;
        int secs = (ms / 1000) % 60;
        int cents = (ms % 1000) / 10;

        if(mins) {
            saynumber(mins, true);
            say(Words::minute);
        }
        saynumber(secs, true);
        say(Words::seconde);
        if(cents) {
            say(Words::et);
            saynumber(cents);
            say(Words::centieme);
        }
    }

    void saysilence(int ms) {
        silence(ms);
    }
};

