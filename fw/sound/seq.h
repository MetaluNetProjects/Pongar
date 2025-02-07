#pragma once

#include <math.h>
#include <string.h>
#include "synth.h"
#include <vector>
#include <algorithm>

struct Harmony {
    using Scale = std::vector<uint8_t>;
    Scale *scale;
    Scale *chord;

    void set_scale(unsigned int i) {
        static Scale scales[3] = {
            {0, 2, 4, 5, 7, 9, 11}, // major
            {0, 2, 3, 5, 7, 8, 10}, // minor nat
            {0, 2, 3, 5, 7, 8, 11}  // minor harm
        };
        i = i % 3;
        scale = &scales[i];
    }

    void set_chord(unsigned int i) {
        static Scale chords[6] = {
            {0, 0, 2, 4},
            {1, 1, 3, 5},
            {2, 2, 4, 6},
            {3, 0, 3, 5},
            {4, 1, 4, 6},
            {5, 0, 2, 5}
        };
        chord = &chords[i % 6];
    }

    static inline int modulo(int a, int b) {
        const int result = a % b;
        return result >= 0 ? result : result + b;
    }
    int get_scale(int n) {
        return 12 * (n / (int)scale->size()) + (*scale)[modulo(n, scale->size())];
    }
    int nearest_chord(int n) {
        int m = modulo(n, (int)scale->size());
        int i;
        if(random() % 2 == 0) for(i = 1; i < (int)chord->size(); i++) {
            if((*scale)[(*chord)[i]] >= m) break;
        }
        else for(i = (int)chord->size() - 1; i <= 1; i--) {
            if((*scale)[(*chord)[i]] <= m) break;
        }
        return scale->size() * (n / (int)scale->size()) + (*chord)[i];
    }
    Harmony() { set_scale(0); set_chord(0);}
    ~Harmony(){}
};

struct Voice {
    SawA synth;
    int octave;
    bool force_chord;
    bool force_base;
    std::vector<int> melody;
    void play_step(int step, int ms, Harmony &harm) {
        if(!melody.size()) return;
        int note = melody[step % melody.size()];
        if(note != -1000) synth.play(note, 10000, ms / 4);
    }
    void make_melody(int steps, Harmony &harm, int silence_percent) {
        melody.clear();
        int note = 0;
        for(int i = 0; i < steps; i++) {
            if(random() % 100 < silence_percent) melody.push_back(-1000);
            else {
                if(force_base) {
                    note = harm.get_scale((*harm.chord)[0]);
                    if(random()%6 == 0) note += 7;
                    if(random()%6 == 0) note += 12;
                    melody.push_back(note + octave * 12 + 2);
                } else {
                    float note_add = (random() % 1000) / 1000.0;
                    note_add *= note_add;
                    note_add = int(note_add * 3.0) + 1;
                    if(random() % 2 == 0) note_add = -note_add;
                    note += note_add;
                    note = CLIP(note, 0, 14);
                    if((i % 4) == 0  || force_chord) note = harm.nearest_chord(note);
                    melody.push_back(harm.get_scale(note) + octave * 12 + 2);
                }
            }
        }
    }
    Voice(int _octave): octave(_octave){}
};

class Sequencer {
  private:
    int step;
    int ms = 400;
    float shuffle = 0.8;
    bool playing = false;
    absolute_time_t next_beat;
    absolute_time_t next_half;

  public:
    Voice v1, v2, v3;
    Harmony harm;
    inline Sequencer() : v1(3), v2(5), v3(5){
        v1.force_base = true;
        //v2.force_chord = true;

        v1.make_melody(16, harm, 70);
        v2.make_melody(16, harm, 30);
        v3.make_melody(16, harm, 20);
    };

    void mix(int32_t *out_buffer) {
        v1.synth.mix(out_buffer);
        v2.synth.mix(out_buffer);
        v3.synth.mix(out_buffer);
        if(playing) {
            if(time_reached(next_beat)) {
                play_step();
                next_beat = make_timeout_time_ms(ms);
                next_half = make_timeout_time_ms(ms / 2 + ms * (shuffle / 6.0));
            }
            if(time_reached(next_half)) {
                play_step();
                next_half = at_the_end_of_time;
            }
        }
    }

    void play_step() {
        if(step % 64 == 0) {
            int c = random() % 6;
            harm.set_chord(c);
            make_melodies();
        }
        if(step % 16 == 0) {
            int v = random() % 10;
            switch(v) {
                case 0: v1.make_melody(16, harm, 70); break;
                case 1: v2.make_melody(16, harm, 30); break;
                case 2: v3.make_melody(16, harm, 20); break;
                default: ;
            }
        }
        v1.play_step(step, ms, harm);
        v2.play_step(step, ms, harm);
        v3.play_step(step, ms, harm);
        step++;
    }

    void make_melodies() {
        v1.make_melody(16, harm, 70);
        v2.make_melody(16, harm, 30);
        v3.make_melody(16, harm, 20);
    }

    void set_shuffle(float s) { shuffle = s; }
    
    void set_tempo_ms(int m) {
        ms = m;
    }
    
    void set_playing(bool p) {
        playing = p;
        if(p) step = 0;
        next_half = at_the_end_of_time;
    }
};
