#pragma once

#include <math.h>
#include <string.h>
#include "osc.h"
#include "filter.h"
#include "enveloppe.h"

#define CLIP(x, min, max) MAX(MIN((x), (max)), (min))

class Synth {
  private:
  public:
    Osc osc1;
    Enveloppe env1;
    //Lop lop1;
    absolute_time_t stop_time;
    int32_t buf[AUDIO_SAMPLES_PER_BUFFER];
    int next_note = 0, next_vol, next_ms;
    int A, S = 1000, R; // millis
    enum Waveform {SIN, SAW, SQUARE} waveform = SAW;
    Synth() : osc1(100, 5000)/*, lop1(500)*/ { osc1.setLfo(400, 150); }
    virtual ~Synth(){}
    virtual void post_process() {}
    void mix(int32_t *out_buffer) {
        if(next_note && env1.is_stopped()) do_play(next_note, next_vol, next_ms);
        if(time_reached(stop_time)) return;
        memset(buf, 0, sizeof(buf));
        switch(waveform) {
            case SIN: osc1.mix_sin(buf); break;
            case SAW: osc1.mix_saw(buf); break;
            case SQUARE: osc1.mix_squ(buf, 10000); break;
        }
        osc1.update();
        //lop1.filter(buf);
        post_process();
        env1.mix_squ(out_buffer, buf);
    }

    virtual void do_play(int note, int vol, int ms) {
        osc1.setMidi(note);
        osc1.setVol(vol);
        //lop1.setMidi(note / 2 + 50);
        int a = MIN(A, ms / 2);
        int r = R; //MIN(R, ms);
        int s = CLIP(ms - a - r, 1, S);
        env1.start(a, s, r);
        stop_time = make_timeout_time_ms(a + s + r + 10);
        next_note = 0;
    }

    void play(int note, int vol, int ms) {
        if(env1.is_stopped()) do_play(note, vol, ms);
        else {
            env1.stop(2);
            next_note = note;
            next_vol = vol;
            next_ms = ms;
        }
    }
    
    void randomize() {
        osc1.setLfo(200 + (random() % 200), random() % 300);
        float a = (random() % 1000) / 1000.0;
        A = a * a * a * 50.0;

        S = random() % 500 + 100;

        float r = (random() % 1000) / 1000.0;
        R = r * r * 800.0;
    }
};

class SynthBp : public Synth {
  private:
  public:
    Hip hip1;
    Bandpass bp1;
    int bpf_offset;
    int bpf_random;
    float bpq;
    SynthBp() : hip1(500), bp1(500, 4, 4) {}
    virtual void post_process() {
        //hip1.filter(buf);
        if(waveform != SIN) bp1.filter(buf);
    }

    virtual void do_play(int note, int vol, int ms) {
        int f = note + bpf_offset + (random() % bpf_random);
        f = CLIP(f, 10, 110);
        bp1.setMidiQ(f, bpq, MAX(bpq * 0.5, 1.0));
        Synth::do_play(note, vol, ms);
    }

    virtual void randomize() {
        Synth::randomize();
        bpf_offset = random() % 36;
        bpf_random = random() % 36 + 1;
        bpq = (random() % 30) + 1.0;
    }
};

