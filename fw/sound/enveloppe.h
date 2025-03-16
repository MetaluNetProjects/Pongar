#pragma once

#include <math.h>
#include <string.h>
#include "sound.h"

#define CLIP(x, min, max) MAX(MIN((x), (max)), (min))

class Enveloppe {
public:
    int Sms;
    int Ainc, Sinc, Rinc;
    int level = 0;
    static const int MAX_LEVEL_BITS = 24;
    static const int MAX_LEVEL = 1 << MAX_LEVEL_BITS;
    enum {OFF, A, S, R} state = OFF;
    absolute_time_t next_time;
    absolute_time_t finish_time;
    void mix(int32_t *out_buffer, int32_t *in_buffer) {
        for (uint i = 0; i < AUDIO_SAMPLES_PER_BUFFER; i++) {
            switch(state) {
            case OFF:
                level = 0;
                break;
            case A:
                level += Ainc;
                if(level >= MAX_LEVEL) {
                    level = MAX_LEVEL;
                    next_time = make_timeout_time_ms(Sms);
                    state = S;
                }
                break;
            case S:
                level = MAX_LEVEL;
                if(time_reached(next_time)) state = R;
                break;
            case R:
                level -= Rinc;
                if(level <= 0) {
                    level = 0;
                    state = OFF;
                }
            }
            *out_buffer++ += (*in_buffer++ * (level >> (MAX_LEVEL_BITS - 10))) / (1 << 10);
        }
    }

    void mix_squ(int32_t *out_buffer, int32_t *in_buffer) {
        for (uint i = 0; i < AUDIO_SAMPLES_PER_BUFFER; i++) {
            switch(state) {
            case OFF:
                level = 0;
                break;
            case A:
                level += Ainc;
                if(level >= MAX_LEVEL) {
                    level = MAX_LEVEL;
                    next_time = make_timeout_time_ms(Sms);
                    state = S;
                }
                break;
            case S:
                level = MAX_LEVEL;
                if(time_reached(next_time)) state = R;
                break;
            case R:
                level -= Rinc;
                if(level <= 0) {
                    level = 0;
                    state = OFF;
                }
            }
            int squlevel = ((level >> (MAX_LEVEL_BITS - 10)) * (level >> (MAX_LEVEL_BITS - 10))) / (1 << 10);
            *out_buffer++ += (*in_buffer++ * squlevel) / (1 << 10);
        }
    }

    void start(int a, int s, int r) {
        if(a < 1) a = 1;
        Ainc = MAX_LEVEL / (a * (AUDIO_SAMPLE_RATE / 1000));
        Sms = s;
        if(r < 1) r = 1;
        Rinc = MAX_LEVEL / (r * (AUDIO_SAMPLE_RATE / 1000));
        level = 0;
        state = A;
        finish_time = make_timeout_time_ms(a + s + r);
    }
    void stop(int r) {
        Rinc = MAX_LEVEL / (r * (AUDIO_SAMPLE_RATE / 1000));
        state = R;
    }
    bool is_stopped() {
        return (state == OFF || time_reached(finish_time));
    }
    float get_level_norm() {
        return ((float)level) / MAX_LEVEL;
    }
};

