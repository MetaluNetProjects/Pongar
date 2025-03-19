#pragma once

#include <math.h>
#include <string.h>
#include "osc.h"
#include "filter.h"
#include "enveloppe.h"
#include "seq.h"
#include "sound_command.h"
#include "patch.h"

#define CLIP(x, min, max) MAX(MIN((x), (max)), (min))

class Buzzer {
private:
public:
    Osc osc1;
    Osc osc2;
    Hip hip1;
    Bandpass bp1;
    absolute_time_t stop_time;
    int32_t buf[AUDIO_SAMPLES_PER_BUFFER];
    int32_t buf2[AUDIO_SAMPLES_PER_BUFFER];
    int gain = 3 * 256;
    int squthres = 0;
    int f1 = 85, f2 = 91;
    int randf = 5;
    Buzzer() : osc1(100, 20000), osc2(103, -20000), hip1(600), bp1(1111, 200, 100) {}
    void mix(int32_t *out_buffer, int32_t *in_buffer = 0) {
        if(time_reached(stop_time)) return;
        memset(buf, 0, sizeof(buf));
        //memset(buf2, 0, sizeof(buf2));
        //osc1.mix_squ(buf, squthres);
        osc1.mix_saw(buf);
        osc2.mix_saw(buf);
        hip1.filter(buf);
        bp1.filter(buf2, buf);
        for (uint i = 0; i < AUDIO_SAMPLES_PER_BUFFER; i++) {
            *out_buffer++ = CLIP(((buf[i] + buf2[i] / 16) * gain) / 256, -65536, 65535);
        }
        osc1.setFreq(f1 + random() % randf);
        osc2.setFreq(f2 + random() % randf);
    }
    void buzz(uint ms) {
        stop_time = make_timeout_time_ms(ms);
    }

    void configOsc(int _f1, int v1, int _f2, int v2, int g) {
        f1 = _f1;
        f2 = _f2;
        osc1.setVol(v1);
        osc1.setVol(v2);
        gain = g * 256;
    }

    void configFilt(int hipf, float f, float q, float g) {
        hip1.setFreq(hipf);
        bp1.setFQ(f, q, g);
    }
};

/*class BuzzerTooClose: public Buzzer {
    BuzzerTooClose(): Buzzer() {
        osc1.setFreq(340);
        osc1.setFreq(343);
        bp1.setFQ(2468, 200, 100);
    }
};*/

class Ring {
public:
    Osc osc1;
    Hip hip1;
    Bandpass bp1, bp2, bp3, bp4;
    absolute_time_t stop_time;
    absolute_time_t finish_time;
    int32_t buf[AUDIO_SAMPLES_PER_BUFFER];
    int32_t buf2[AUDIO_SAMPLES_PER_BUFFER];
    Ring(): osc1(30, 10000), hip1(500),
        bp1(1197, 1000, 1000),
        bp2(2814, 1500, 1500),
        bp3(4822, 2000, 2000),
        bp4(7292, 2000, 2000)
    {}
    void mix(int32_t *out_buffer, int32_t *in_buffer = 0) {
        if(time_reached(finish_time)) return;
        memset(buf, 0, sizeof(buf));
        //memset(buf2, 0, sizeof(buf2));
        if(!time_reached(stop_time)) {
            osc1.mix_saw(buf);
            hip1.filter(buf);
            for (uint i = 0; i < AUDIO_SAMPLES_PER_BUFFER; i++) {
                buf[i] = (buf[i] * (random() % 1024)) / 1024;
            }
        }
        //for(int i = 0; i < 4; i++) bp[i].mix(buf2, buf);
        bp1.filter(buf2, buf);
        bp2.mix(buf2, buf);
        bp3.mix(buf2, buf);
        bp4.mix(buf2, buf);
        for (uint i = 0; i < AUDIO_SAMPLES_PER_BUFFER; i++) {
            *out_buffer++ += CLIP(buf2[i] / 4, -32768, 32767);
        }
    }
    void ring(int ms) {
        stop_time = make_timeout_time_ms(ms);
        finish_time = make_timeout_time_ms(ms + 500);
    }
};

class Tut {
private:
public:
    Osc osc1;
    absolute_time_t stop_time;
    Tut() : osc1(1000, 20000) {}
    void mix(int32_t *out_buffer, int32_t *in_buffer = 0) {
        if(time_reached(stop_time)) return;
        osc1.mix_saw(out_buffer);
    }
    void tut(uint f, uint ms) {
        osc1.setFreq(f);
        stop_time = make_timeout_time_ms(ms);
    }
};

class Bouncer {
private:
    Osc osc1;
    Enveloppe env;
    int freq;
    int freq_inc;
public:
    absolute_time_t stop_time;
    Bouncer() : osc1(500, 20000) {}
    void mix(int32_t *out_buffer, int32_t *in_buffer = 0) {
        if(time_reached(stop_time)) return;
        int32_t buf[AUDIO_SAMPLES_PER_BUFFER] = {0};
        osc1.mix_sin(buf);
        env.mix(out_buffer, buf);
        freq += freq_inc;
        osc1.setFreq8(freq);
    }
    void bounce(uint ms, int f, int df) {
        env.start(5, ms, ms / 2);
        freq = f * 256;
        freq_inc = (df * 256 * AUDIO_SAMPLES_PER_BUFFER) / AUDIO_SAMPLE_RATE;
        osc1.setFreq8(f);
        stop_time = make_timeout_time_ms(ms * 2);
    }
};

class MainPatch: public Patch {
public:
    Buzzer buzzer;
    Buzzer buzzer_too_close;
    Bouncer bouncer;
    Tut tut;
    Ring ring;
    Sequencer seq;
    Blosc osc1;
    enum WF {SIN, SAW, SQU, BLSAW, BLSQU} osc1_waveform = BLSAW;
    MainPatch() {
        buzzer_too_close.configOsc(340, 20000, 343, -20000, 3);
        buzzer_too_close.configFilt(600, /*2468*/ 1867, 200, 200);
        buzzer_too_close.randf = 50;
    }
    virtual void mix(int32_t *out_buffer, int32_t *in_buffer = 0) {
        bouncer.mix(out_buffer);
        tut.mix(out_buffer);
        seq.mix(out_buffer); // seq adds reverb!
        ring.mix(out_buffer);
        buzzer.mix(out_buffer);
        buzzer_too_close.mix(out_buffer);
        switch(osc1_waveform) {
        case SIN:
            osc1.mix_sin(out_buffer);
            break;
        case SAW:
            osc1.mix_saw(out_buffer);
            break;
        case SQU:
            osc1.mix_squ(out_buffer, 20000);
            break;
        case BLSAW:
            osc1.mix_blsaw(out_buffer);
            break;
        case BLSQU:
            osc1.mix_blsqu(out_buffer, 20000);
            break;
        }
    }
    void buzz() {
        buzzer.buzz(400);
    }
    void bounce(bool way = true) {
        if(way) bouncer.bounce(120, 200, 5000);
        else bouncer.bounce(120, 1000, -3000);
    }

    virtual void command(SoundCommand c, int p1, int p2, int p3) {
        switch(c) {
        case SoundCommand::buzz:
            buzzer.buzz(p1);
            break;
        case SoundCommand::tooclose:
            buzzer_too_close.buzz(p1);
            break;
        case SoundCommand::bounce:
            bounce(p1 > 0);
            break;
        case SoundCommand::tut:
            tut.tut(p1, p2);
            break;
        case SoundCommand::ring:
            ring.ring(p1);
            break;
        //case SoundCommand::note: seq.v1.synth.play(p1, p2, p3); break;
        //case SoundCommand::lfoA: seq.v1.synth.osc1.setLfo(p1, p2); break;
        case SoundCommand::seqnew:
            seq.make_new_piece();
            break;
        case SoundCommand::seqplay:
            seq.set_playing(p1, p2);
            break;
        case SoundCommand::seqms:
            seq.set_tempo_ms(p1);
            break;
        case SoundCommand::seqvol:
            seq.set_mastervol(p1);
            break;
        case SoundCommand::osc1:
            osc1.setVol(p1);
            osc1.setFreq8(p2);
            osc1.set_bandlimit(p3 / 256.0);
            break;
        case SoundCommand::osc1wf:
            osc1_waveform = (WF)p1;
            break;
       case SoundCommand::rev1:
            seq.config_reverb(p1, p2);
            break;
        default:
            ;
        }
    }
};
