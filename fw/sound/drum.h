#pragma once

#include <math.h>
#include <string.h>
#include "osc.h"
#include "filter.h"
#include "enveloppe.h"

#define CLIP(x, min, max) MAX(MIN((x), (max)), (min))

class Drum {
private:
protected:
    Enveloppe env1;
    absolute_time_t stop_time;
    int32_t buf[AUDIO_SAMPLES_PER_BUFFER] = {0};
    int next_vol = 0, next_ms;
    int A = 1, S = 10, R = 200; // millis
    int volume = 0;
    Drum() {}
public:
    virtual ~Drum() {}
    virtual void process() {}
    void mix(int32_t *out_buffer) {
        if(next_vol && env1.is_stopped()) do_play(next_vol, next_ms);
        if(time_reached(stop_time)) return;
        memset(buf, 0, sizeof(buf));
        process();
        env1.mix_squ(out_buffer, buf);
    }

    virtual void do_play(int vol, int ms) {
        env1.start(A, S, R);
        stop_time = make_timeout_time_ms(A + S + R);
        volume = vol;
        next_vol = 0;
    }

    void play(int vol, int ms) {
        if(env1.is_stopped()) do_play(vol, ms);
        else {
            env1.stop(2);
            next_vol = vol;
            next_ms = ms;
        }
    }

    float randf(float amp = 1.0) {
        return amp * (random() % 1024) / 1024.0;
    }

    virtual void randomize() {
    }
};

class Hihat : public Drum {
private:
    Bandpass bp1;
    int bpf_offset = 0;
    int bpf_random = 0;
    float bpf_current = 0;
    float bpf_dest = 0;
    float bpf_portamento = 0;
    float bpq = 0;
public:
    Hihat() : bp1(8000, 30, 8) {
        R = 100;
    }
    virtual void process() {
        if(volume) for(int i = 0; i < AUDIO_SAMPLES_PER_BUFFER; i++) buf[i] = (random() % (volume * 2)) - volume;
        bpf_current += (bpf_dest - bpf_current) * (1.0 - bpf_portamento);
        bp1.setMidiQ(bpf_current, bpq, bpq * 0.2 + 1.0);
        bp1.filter(buf);
    }

    virtual void do_play(int vol, int ms) {
        int f = bpf_offset + (random() % bpf_random);
        bpf_dest = CLIP(f, 10, 134);
        Drum::do_play(vol, ms);
    }

    virtual void randomize() {
        Drum::randomize();
        R = 20 + (random() % 150);
        bpf_offset = 105 + (random() % 30);
        bpf_random = (random() % 12) + 1;
        bpq = (random() % 20) + 15.0;
        float Tmax = 0.5; // seconds
        float portamento_max = 1.0 - (6.28 * AUDIO_SAMPLES_PER_BUFFER) / (Tmax * AUDIO_SAMPLE_RATE);
        bpf_portamento = 0.1 + randf(0.9 * portamento_max);
    }
};

class Snare : public Drum {
private:
    Bandpass bp1;
    int bpf_offset = 0;
    int bpf_random = 0;
    float bpf_current = 0;
    float bpf_dest = 0;
    float bpf_portamento = 0;
    float bpq = 0;
public:
    Snare() : bp1(8000, 30, 8) {
        R = 100;
    }
    virtual void process() {
        if(volume) for(int i = 0; i < AUDIO_SAMPLES_PER_BUFFER; i++) buf[i] = (random() % (volume * 2)) - volume;
        bpf_current += (bpf_dest - bpf_current) * (1.0 - bpf_portamento);
        bp1.setMidiQ(bpf_current, bpq, bpq * 0.2 + 1.0);
        bp1.filter(buf);
    }

    virtual void do_play(int vol, int ms) {
        int f = bpf_offset + (random() % bpf_random);
        bpf_dest = CLIP(f, 10, 134);
        Drum::do_play(vol, ms);
    }

    virtual void randomize() {
        Drum::randomize();
        R = 50 + (random() % 450);
        bpf_offset = 82 + (random() % 10);
        bpf_random = random() % 6 + 1;
        bpq = (random() % 5) + 5.0;
        float Tmax = 0.5; // seconds
        float portamento_max = 1.0 - (6.28 * AUDIO_SAMPLES_PER_BUFFER) / (Tmax * AUDIO_SAMPLE_RATE);
        bpf_portamento = 0.1 + randf(0.9 * portamento_max);
    }
};

class Kick : public Drum {
private:
    Osc osc1;
    float note_max = 55;
    float note_min = 20;
public:
    Kick() : osc1(100, 5000) {
        R = 1000;
    }
    virtual void process() {
        int note = env1.get_level_norm() * (note_max - note_min) + note_min;
        osc1.setFreq8(Osc::mtof8(note));
        osc1.mix_sin(buf);
    }

    virtual void do_play(int vol, int ms) {
        //printf("kick do_play vol=%d\n", vol);
        osc1.setVol(vol);
        Drum::do_play(vol, ms);
    }

    virtual void randomize() {
        Drum::randomize();
        R = 200 + (random() % 300);
        note_max = 44 + (random() % 15);
    }
};

