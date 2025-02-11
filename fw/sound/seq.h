#pragma once

#include <math.h>
#include <string.h>
#include "synth.h"
#include <vector>
#include <array>
#include <algorithm>

using Scale = std::vector<uint8_t>;
using Melody = std::vector<int8_t>;

struct Harmony {
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
    SynthBp synth;
    int octave = 5;
    int silence_percent = 50;
    bool force_chord;
    bool force_base;
    int note_offset;
    int volume = 5000;
    void play_step(int step, int ms, Melody &melody) {
        if(!melody.size()) return;
        int note = melody[step % melody.size()];
        if(note != -128) synth.play(note + note_offset, volume, ms / 2);
    }
    Melody make_melody(int steps, Harmony &harm) {
        Melody melody;
        int note = 0;
        for(int i = 0; i < steps; i++) {
            if(random() % 100 < silence_percent) melody.push_back(-128);
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
        //printf("melody: "); for(int i = 0 ; i < melody.size(); i++) printf("%d ", melody[i]); printf("\n");
        return melody;
    }
    void randomize(bool is_bass) {
        force_base = is_bass;
        if(!is_bass) {
            synth.waveform = (Synth::Waveform)(random() % 3);
            force_chord = ((random() % 4) == 0);
            octave = 5 + (random() % 3);
            if(octave > 6) synth.waveform = Synth::SIN;
            silence_percent = (random() % 40) + 10;
        } else {
            synth.waveform = (Synth::Waveform)((random() % 2) + 1);
            volume = 10000;
            octave = 3 + (random() % 2);
            silence_percent = 30 + (random() % 35);
        }
        synth.randomize();
    }
};

#define SEQUENCER_NB_VOICES 3

class Part {
    Melody melodies[SEQUENCER_NB_VOICES];
  public:
    Harmony harm;
    void make_melodies(Voice *voices) {
        for(int i = 0; i < SEQUENCER_NB_VOICES; i++) melodies[i] = voices[i].make_melody(16, harm);
    }
    void play_step(int step, int ms, Voice *voices) {
        for(int i = 0; i < SEQUENCER_NB_VOICES; i++) voices[i].play_step(step, ms, melodies[i]);
    }
};

class Piece {
    std::array<Part, 8> parts;
    std::vector<int> plan;
    int plan_index = 0;
  public:
    Piece(){}
    ~Piece(){}
    void make(Voice *voices) {
        int note_offset = (random() % 12) - 6;
        for(int i = 0; i < SEQUENCER_NB_VOICES; i++) {
            voices[i].note_offset = note_offset;
            voices[i].randomize(i == 0);
        }
        int scale = random() % 5;
        if(scale > 2) scale = 0;
        parts[0].harm.set_scale(scale);
        parts[0].harm.set_chord(0);
        parts[0].make_melodies(voices);
        parts[1].harm.set_scale(scale);
        parts[1].harm.set_chord(3);
        parts[1].make_melodies(voices);
        parts[2].harm.set_scale(scale);
        parts[2].harm.set_chord(4);
        parts[2].make_melodies(voices);

        plan = {0, 0, 1, 1, 0, 0, 2, 1};
    }

    void play_step(int step, int ms, Voice *voices) {
        if(parts.empty() || plan.empty()) return;
        plan_index = (step / 16) % plan.size();
        int current_part = plan[plan_index] % parts.size();
        parts[current_part].play_step(step, ms, voices);
    }
};

class Sequencer {
  private:
    int step;
    int ms = 300;
    float shuffle = 0.5;
    bool playing = false;
    absolute_time_t next_beat;
    absolute_time_t next_half;

  public:
    Voice voices[SEQUENCER_NB_VOICES];
    Piece piece;
    inline Sequencer() {
        piece.make(voices);
    };

    void mix(int32_t *out_buffer) {
        for(int i = 0; i < SEQUENCER_NB_VOICES; i++) voices[i].synth.mix(out_buffer);
        if(playing) {
            if(time_reached(next_beat)) {
                play_step();
                next_beat = make_timeout_time_ms(ms);
                next_half = make_timeout_time_ms(ms / 2 + ms * (shuffle / 4.0));
            }
            if(time_reached(next_half)) {
                play_step();
                next_half = at_the_end_of_time;
            }
        }
    }

    void play_step() {
        piece.play_step(step, ms, voices);
        step++;
    }

    void set_shuffle(float s) { shuffle = s; }
    
    void set_tempo_ms(int m) {
        ms = m;
    }
    
    void set_playing(bool p) {
        playing = p;
        if(p) {
            step = 0;
            piece.make(voices);
            set_shuffle((random() % 1000) / 1000.0);
        }
        next_half = at_the_end_of_time;
    }
};
