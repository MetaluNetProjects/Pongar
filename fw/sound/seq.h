#pragma once

#include <math.h>
#include <string.h>
#include "synth.h"
#include "drum.h"
#include "echo.h"
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
    Harmony() {
        set_scale(0);
        set_chord(0);
    }
    ~Harmony() {}
};

class Voice {
private:
    SynthBp synth;
public:
    int octave = 5;
    int silence_percent = 50;
    bool force_chord;
    bool force_base;
    int note_offset;
    int volume = 5000;
    void play_step(int step, int ms, Melody &melody, uint8_t mastervol) {
        if(!melody.size()) return;
        int note = melody[step % melody.size()];
        if(note != -128) synth.play(note + note_offset, (volume * mastervol) / 256, ms / 2);
    }
    Melody make_melody(int steps, Harmony &harm) {
        Melody melody;
        int note = 0;
        int final_steps = steps;
        const int chances = 6;
        if(random() % chances == 0) final_steps = 6;
        if(random() % chances == 0) final_steps = 8;
        if(random() % chances == 0) final_steps = 12;
        if(random() % chances == 0) final_steps = 4;
        for(int i = 0; i < final_steps; i++) {
            if(random() % 100 < silence_percent) melody.push_back(-128);
            else {
                if(force_base) {
                    note = harm.get_scale((*harm.chord)[0]);
                    if(random() % 6 == 0) note += 7;
                    if(random() % 6 == 0) note += 12;
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
            octave = 4 + (random() % 3);
            silence_percent = (random() % 40) + 10;
            volume = 5000;
        } else {
            synth.waveform = (Synth::Waveform)((random() % 2) + 1);
            volume = 11000;
            octave = 2 + (random() % 3);
            silence_percent = 30 + (random() % 35);
        }
        synth.randomize();
    }
    void mix(int32_t *out_buffer) {
        synth.mix(out_buffer);
    }
};

class Drumvoice {
private:
    class PlayProba {
    private:
        int steps;
        int wanted;
        int percent;
    public:
        PlayProba(int _steps, int _wanted, int _percent): steps(_steps), wanted(_wanted), percent(_percent) {}
        bool get(int step) {
            if((step % steps) == wanted) return (random() % 100) < percent;
            else return false;
        }
    };
    std::vector<PlayProba> probas;
    int8_t volume = 20; // 0 - 127
    Drum *drum;
public:
    Drumvoice(Drum *_drum): drum(_drum) {};
    void set_volume(int8_t v) {
        volume = v;
    }
    void clear_probas() {
        probas.clear();
    }
    void add_proba(int steps, int wanted, int percent) {
        probas.emplace_back(steps, wanted, percent);
    }
    void play_step(int step, int ms, Melody &pattern, uint8_t mastervol) {
        if(!pattern.size()) return;
        int8_t vol = pattern[step % pattern.size()];
        if(vol != -128) drum->play(vol * mastervol, ms);
    }
    Melody make_pattern(int steps) {
        Melody pattern;
        for(int i = 0; i < steps; i++) {
            bool play = false;
            for(auto &p : probas) {
                play |= p.get(i);
                if(play) break;
            }
            if(play) pattern.push_back(volume);
            else pattern.push_back(-128);
        }
        return pattern;
    }
    void mix(int32_t *out_buffer) {
        drum->mix(out_buffer);
    }
    void randomize_drum() {
        drum->randomize();
    }
};

class Reverb {
private:
    Echo<5011> echo1;
    Echo<7529> echo2;
    int32_t buf[AUDIO_SAMPLES_PER_BUFFER];
    int32_t lastbuf[AUDIO_SAMPLES_PER_BUFFER];
    uint16_t feedback = 3000;
    uint16_t volume = 5000;
    int16_t lop_last;

public:
    Reverb() {
        echo1.config(0, 32767, 5011);
        echo2.config(0, 32767, 7529);
    }
    void mix(int32_t *out_buffer) {
        for(int i = 0; i < AUDIO_SAMPLES_PER_BUFFER; i++) {
            buf[i] = (volume * out_buffer[i] + (lastbuf[i] + lop_last) * feedback) >> 15;
            lop_last = lastbuf[i];
        }
        echo1.filter(lastbuf, buf);
        echo2.mix(lastbuf, buf);
        for(int i = 0; i < AUDIO_SAMPLES_PER_BUFFER; i++) {
            out_buffer[i] += lastbuf[i];
        }
    }
    void config(uint16_t _feedback, uint16_t _volume) {
        feedback = _feedback / 2;
        volume = _volume;
    }
};

class Piece {
private:
    static const int NB_VOICES = 4;
    static const int NB_DRUMS = 3;
    class Part {
        Melody melodies[NB_VOICES];
        Melody patterns[NB_DRUMS];
    public:
        ~Part() {}
        Harmony harm;
        void make_melodies(Voice *voices, Drumvoice *drumvoices) {
            for(int i = 0; i < NB_VOICES; i++) melodies[i] = voices[i].make_melody(16, harm);
            for(int i = 0; i < NB_DRUMS; i++) patterns[i] = drumvoices[i].make_pattern(16);
        }
        void play_step(int step, int ms, Voice *voices, Drumvoice *drumvoices, uint8_t mastervol) {
            for(int i = 0; i < NB_VOICES; i++) voices[i].play_step(step, ms, melodies[i], mastervol);
            for(int i = 0; i < NB_DRUMS; i++) drumvoices[i].play_step(step, ms, patterns[i], mastervol);
        }
    };

    std::array<Part, 6> parts;
    std::vector<int> plan;
    std::vector<std::vector<int>> plans = {
        {0, 0, 3, 3, 0, 0, 4, 3},
        {0, 3, 0, 4},
        {5, 0, 1, 3, 5, 0, 2, 2, 5, 0, 1, 3, 5, 2, 5, 2},
        {0, 1, 2, 3, 0, 2, 3, 4},
        {0, 2, 0, 1, 0, 4, 0, 3, 4, 0, 1, 2, 0, 2, 3, 4},
        {0, 0, 3, 3, 2, 2, 4, 4},
        {0, 3, 1, 4, 0, 3, 2, 4}
    };
    int plan_index = 0;
    int plan_steps = 16;
    Voice voices[NB_VOICES];
    Hihat hh;
    Snare snare;
    Kick kick;
    Drumvoice drumvoices[NB_DRUMS] = {&hh, &snare, &kick};
    enum drumnames {HH = 0, SNARE, KICK};
    Reverb rev1;
    uint8_t mastervol = 150;
public:
    Piece() {}
    ~Piece() {}
    void randomize_reverb() {
        config_reverb(random() % 8000, random() % 16000);
    }
    void randomize_voices() {
        int note_offset = (random() % 12) - 6;
        for(int i = 0; i < NB_VOICES; i++) {
            voices[i].note_offset = note_offset;
            voices[i].randomize(i == 0);
        }
    }
    void randomize_drumvoices() {
        for(int i = 0; i < NB_DRUMS; i++) {
            drumvoices[i].randomize_drum();
            drumvoices[i].clear_probas();
        }
        drumvoices[HH].add_proba(4, 0, random() % 100);
        drumvoices[HH].add_proba(4, 1, random() % 30);
        drumvoices[HH].add_proba(4, 2, random() % 100);
        drumvoices[HH].add_proba(4, 3, random() % 100);
        drumvoices[HH].set_volume(5 + (random() % 15));

        drumvoices[SNARE].add_proba(8, 4, random() % 80);
        drumvoices[SNARE].add_proba(1, 0, random() % 20);
        drumvoices[SNARE].set_volume(40 + (random() % 40));

        int kick_proba = random() % 100;
        drumvoices[KICK].add_proba(16, 0, kick_proba);
        drumvoices[KICK].add_proba(8, 0, kick_proba * (random() % 2 == 0));
        drumvoices[KICK].add_proba(4, 0, kick_proba * (random() % 4 == 0));
        drumvoices[KICK].add_proba(1, 0, (kick_proba / 10) * (random() % 4 == 0));
        drumvoices[KICK].set_volume(60 + (random() % 50));
    }
    void make() {
        randomize_voices();
        randomize_drumvoices();
        randomize_reverb();

        int scale = random() % 5;
        if(scale > 2) scale = 0;
        for(int i = 0; i < (int)parts.size(); i++) {
            parts[i].harm.set_chord(i);
            parts[i].harm.set_scale(scale);
            parts[i].make_melodies(voices, drumvoices);
        }
        plan = plans[random() % plans.size()];
        plan_steps = 16;
        if(random() % 4 == 0) plan_steps = 8;
        if(random() % 6 == 0) plan_steps = 12;
    }

    void play_step(int step, int ms) {
        if(parts.empty() || plan.empty()) return;
        plan_index = (step / plan_steps) % plan.size();
        int current_part = plan[plan_index] % parts.size();
        parts[current_part].play_step(step, ms, voices, drumvoices, mastervol);
    }

    void mix(int32_t *out_buffer) {
        for(int i = 0; i < NB_VOICES; i++) voices[i].mix(out_buffer);
        rev1.mix(out_buffer);
        for(int i = 0; i < NB_DRUMS; i++) drumvoices[i].mix(out_buffer);
    }
    void config_reverb(uint16_t feedback, uint16_t volume) {
        rev1.config(feedback, volume);
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
    Piece piece;
public:
    inline Sequencer() {
        piece.make();
    };

    void mix(int32_t *out_buffer) {
        piece.mix(out_buffer);
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
        piece.play_step(step, ms);
        step++;
    }

    void set_shuffle(float s) {
        shuffle = s;
    }

    void set_tempo_ms(int m) {
        ms = m;
    }

    void set_playing(bool p) {
        playing = p;
        if(p) step = 0;
        next_half = at_the_end_of_time;
    }

    void make_new_piece() {
        piece.make();
        shuffle = (random() % 1000) / 1000.0;
        shuffle *= shuffle;
    }

    void config_reverb(uint16_t feedback, uint16_t volume) {
        piece.config_reverb(feedback, volume);
    }

};
