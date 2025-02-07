#pragma once

#include <math.h>
#include <string.h>
#include "osc.h"
#include "filter.h"
#include "enveloppe.h"

#define CLIP(x, min, max) MAX(MIN((x), (max)), (min))

class SawA {
  private:
  public:
    Osc osc1;
    Enveloppe env1;
    Lop lop1;
    absolute_time_t stop_time;
    int32_t buf[AUDIO_SAMPLES_PER_BUFFER];
    int next_note = 0, next_vol, next_ms;
    SawA() : osc1(100, 10000), lop1(500) { osc1.setLfo(/*280, 100*/ 400, 150); }
    void mix(int32_t *out_buffer, int32_t *in_buffer = 0) {
        if(next_note && env1.is_stopped()) do_play(next_note, next_vol, next_ms);
        if(time_reached(stop_time)) return;
        memset(buf, 0, sizeof(buf));
        osc1.mix_saw(buf);
        lop1.filter(buf);
        env1.mix_squ(out_buffer, buf);
        osc1.update();
    }

    void do_play(int note, int vol, int ms) {
        osc1.setMidi(note);
        osc1.setVol(vol);
        lop1.setMidi(note / 2 + 50);
        env1.start(5, ms, ms / 4);
        stop_time = make_timeout_time_ms(5 + 2 * ms);
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

};

