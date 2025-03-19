// global game

#pragma once
#include "wavplayer.h"
#include "words.h"
#include <vector>

class Speaker: public WavPlayer {
private:
    int mode = 2;
    bool exists[256];
    std::vector<uint16_t> alpagues;
public:
    static const bool FEMALE = true;
    static const bool MALE = false;

    void init(int tx_pin) {
        WavPlayer::init(tx_pin);
        set_mode(3);
    }

    virtual void reload_durations() {
        set_mode(mode);
    }

    void set_mode(int m) {
        mode = m;
        for(int i = 0; i < 256; i++) exists[i] = get_duration_ms(mode, i) != 0;
        int alpague = (int)Words::alpague;
        alpagues.clear();
        for(uint8_t bank = 0; bank < 3; bank++)
            for(uint8_t num = 0; num < 5; num++)
                if(get_duration_ms(mode, alpague + bank * 5 + num))
                    alpagues.push_back(alpague + bank * 5 + num);
    }

    void say(Words w, int offset = -1) {
        int word = (int)w;
        int final_offset = offset;
        if(offset == -1) {
            std::vector<uint8_t> existing;
            for(int i = 0; i < 5; i++) if(exists[word + i]) existing.push_back(i);
            if(!existing.size()) return;
            final_offset = existing[random() % existing.size()];
        }
        play(mode, word + final_offset);
    }

    void saynumber(int n, bool gender = MALE) {
        bool say_zero = true;
        if(n >= 1000) {
            int m = n / 1000;
            n %= 1000;
            m = m % 1000;
            if(m > 1) saynumber(m, gender);
            say(Words::_1000, 0);
            say_zero = false;
        }
        if(n >= 100) {
            int m = n / 100;
            n %= 100;
            if(m > 1) saynumber(m, gender);
            say(Words::_100, 0);
            say_zero = false;
        }
        if(n >= 21) {
            int m = n / 10;
            if(m == 7) m = 6;
            else if(m == 9) m = 8;
            n -= m * 10;
            say(Words::_20, m - 2);
            if((n == 1 || n == 11) && m != 8) say(Words::et, 0);
            if(n != 0) saynumber(n, gender);
        }
        else {
            if(n == 0 && !say_zero) return;
            if(n == 1 && gender == FEMALE) say(Words::une, 0);
            else say(Words::_0, n);
        }
    }

    void say_hundredths(float f) {
        int n = (int)f;
        int m = (f - (float)n) * 100;
        saynumber(n);
        if(m) {
            say(Words::et, 0);
            saynumber(m);
            say(Words::centieme, 0);
        }
    }

    void say_time(int ms) {
        int mins = (ms / 1000) / 60;
        int secs = (ms / 1000) % 60;
        int cents = (ms % 1000) / 10;

        if(mins) {
            saynumber(mins, true);
            say(Words::minute, 0);
        }
        if(secs != 0) {
            saynumber(secs, true);
            say(Words::seconde, 0);
        }
        if(cents) {
            if(mins != 0 || secs != 0) say(Words::et, 0);
            saynumber(cents);
            say(Words::centieme, 0);
        }
    }

    void say_alpague() {
        if(!alpagues.size()) return;
        uint16_t num = alpagues[random() % alpagues.size()];
        play(mode, num);
    }
};

