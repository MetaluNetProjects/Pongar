#pragma once

#include <math.h>
#include <string.h>
#include "osc.h"
#include "filter.h"
#include "enveloppe.h"

#define CLIP(x, min, max) MAX(MIN((x), (max)), (min))

class Synth : public Blosc{
private:
protected:
    Enveloppe env1;
    absolute_time_t stop_time;
    int32_t buf[AUDIO_SAMPLES_PER_BUFFER];
    int next_note = 0, next_vol, next_ms, next_sustain_ms;
    int A, S = 1000, R; // millis
    float portamento;
    float note_current;
    float note_dest;
public:
    enum Waveform {SIN, SAW, SQUARE} waveform = SAW;
    Synth() {}
    virtual ~Synth() {}
    virtual void post_process() {}
    void mix(int32_t *out_buffer) {
        if(next_note && env1.is_stopped()) do_play(next_note, next_vol, next_ms, next_sustain_ms);
        if(time_reached(stop_time)) return;
        note_current += (note_dest - note_current) * (1.0 - portamento);
        setFreq8(Osc::mtof8(note_current));
        memset(buf, 0, sizeof(buf));
        switch(waveform) {
        case SIN:
            mix_sin(buf);
            break;
        case SAW:
            mix_blsaw(buf);
            break;
        case SQUARE:
            mix_blsqu(buf, 10000);
            break;
        }
        update();
        post_process();
        env1.mix_squ(out_buffer, buf);
    }

    virtual void do_play(int note, int volume, int ms, int sustain_ms) {
        note_dest = note;
        setVol(volume);
        int a = MIN(A, ms / 2);
        int r = R;
        int s = MAX(sustain_ms, CLIP(ms - a - r, 1, S));
        env1.start(a, s, r);
        stop_time = make_timeout_time_ms(a + s + r + 10);
        next_note = 0;
    }

    void play(int note, int volume, int ms, int sustain_ms = 0) {
        if(env1.is_stopped()) do_play(note, volume, ms, sustain_ms);
        else {
            env1.stop(2);
            next_note = note;
            next_vol = volume;
            next_ms = ms;
            next_sustain_ms = sustain_ms;
        }
    }

    float randf(float amp = 1.0) {
        return amp * (random() % 1024) / 1024.0;
    }

    void randomize() {
        setLfo(4 + randf(4), randf(0.2));
        float a = randf();
        A = a * a * a * 50.0;

        S = (random() % 500) + 200;

        float r = randf();
        R = r * r * 800.0;

        float Tmax = 0.1; // seconds
        float portamento_max = 1.0 - (6.28 * AUDIO_SAMPLES_PER_BUFFER) / (Tmax * AUDIO_SAMPLE_RATE);
        portamento = randf(portamento_max);
    }

    void set_asr_ms(int a, int s, int r) {
        A = a;
        S = s;
        R = r;
    }

    void set_wf_lfo_porta(int wf, float lfo_f, float lfo_a, float porta_ms) {
        waveform = (Waveform)(wf % 3);
        setLfo(lfo_f, lfo_a);
        portamento = 1.0 - (6.28 * AUDIO_SAMPLES_PER_BUFFER) / (porta_ms / 1000.0 * AUDIO_SAMPLE_RATE);
    }
};

class SynthBp : public Synth {
private:
    Hip hip1;
    Bandpass bp1;
    int bpf_offset;
    int bpf_random;
    float bpf_current;
    float bpf_dest;
    float bpf_portamento;
    float bpq;
public:
    SynthBp() : hip1(500), bp1(500, 4, 4) {}
    virtual void post_process() {
        //hip1.filter(buf);
        bpf_current += (bpf_dest - bpf_current) * (1.0 - bpf_portamento);
        bp1.setMidiQ(bpf_current, bpq, bpq * 0.1 + 1.0);
        if(waveform != SIN) bp1.filter(buf);
    }

    virtual void do_play(int note, int volume, int ms, int sustain_ms) {
        int f = note + bpf_offset + (random() % bpf_random);
        bpf_dest = CLIP(f, 10, 110);
        Synth::do_play(note, volume, ms, sustain_ms);
    }

    void randomize() {
        Synth::randomize();
        bpf_offset = 15 + (random() % 24);
        bpf_random = (random() % 36) + 1;
        bpq = (random() % 20) + 1.0;
        float Tmax = 0.5; // seconds
        float portamento_max = 1.0 - (6.28 * AUDIO_SAMPLES_PER_BUFFER) / (Tmax * AUDIO_SAMPLE_RATE);
        bpf_portamento = 0.1 + randf(0.9 * portamento_max);
    }

    void set_filter(int f, int f_rnd, float q, int porta_ms) {
        bpf_offset = f;
        bpf_random = MAX(f_rnd, 1);
        bpq = q;
        bpf_portamento = 1.0 - (6.28 * AUDIO_SAMPLES_PER_BUFFER) / (porta_ms / 1000.0 * AUDIO_SAMPLE_RATE);
    }
};

