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

class Voice: public SynthBp {
public:
    int octave = 5;
    int silence_percent = 50;
    bool force_chord;
    bool force_base;
    int note_offset;
    int volume = 5000;
    void play_step(int step, int ms, Melody &melody, uint8_t mastervol, int sustain_ms) {
        if(!melody.size()) return;
        int note = melody[step % melody.size()];
        if(note != -128) play(note + note_offset, (volume * mastervol) / 256, ms / 2, sustain_ms);
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
            waveform = (Waveform)(random() % 3);
            force_chord = ((random() % 4) == 0);
            octave = 4 + (random() % 3);
            silence_percent = (random() % 40) + 10;
            volume = 5000;
        } else {
            waveform = (Waveform)((random() % 2) + 1);
            volume = 11000;
            octave = 2 + (random() % 3);
            silence_percent = 30 + (random() % 35);
        }
        SynthBp::randomize();
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

class Piece {
public:
    using plan_t = std::vector<int>;
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
        void play_step(int step, int ms, Voice *voices, Drumvoice *drumvoices, uint8_t mastervol, int sustain_ms) {
            for(int i = 0; i < NB_VOICES; i++) voices[i].play_step(step, ms, melodies[i], mastervol, sustain_ms);
            if(drumvoices) for(int i = 0; i < NB_DRUMS; i++) drumvoices[i].play_step(step, ms, patterns[i], mastervol);
        }
    };
    std::array<Part, 6> parts;
    plan_t plan;
    std::vector<plan_t> plans = {
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
    uint8_t mastervol = 160;
    bool play_once = true;
    int sustain_ms = 1000;
public:
    Piece() {}
    ~Piece() {}
    void randomize_reverb() {
        float rev_feedback = (random() % 10000) / 10000.0;
        rev_feedback *= rev_feedback;
        float rev_vol = (random() % 10000) / 10000.0;
        rev_vol *= rev_vol;
        config_reverb(rev_feedback * 8000, rev_vol * 16000);
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

    void make(int scale = -1) {
        randomize_voices();
        randomize_drumvoices();
        randomize_reverb();

        if(scale == -1) {
            scale = random() % 5;
            if(scale > 2) scale = 0;
        }

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

    void set_plan(int steps, plan_t &p) {
        plan = p;
        plan_steps = steps;
    }

    void play_step(int step, int ms, bool play_drums) {
        if(parts.empty() || plan.empty()) return;
        if(finished(step)) return;
        plan_index = (step / plan_steps) % plan.size();
        int current_part = plan[plan_index] % parts.size();
        int sustain = 0;
        if(play_once && (step > get_total_steps() - 1)) sustain = sustain_ms;
        parts[current_part].play_step(step, ms, voices, play_drums ? drumvoices : NULL, mastervol, sustain);
    }

    void mix(int32_t *out_buffer) {
        for(int i = 0; i < NB_VOICES; i++) voices[i].mix(out_buffer);
        rev1.mix(out_buffer);
        for(int i = 0; i < NB_DRUMS; i++) drumvoices[i].mix(out_buffer);
    }
    void config_reverb(uint16_t feedback, uint16_t volume) {
        rev1.config(feedback, volume);
    }
    void set_once(bool once) {
        play_once = once;
    }
    bool finished(int step) {
        return play_once && (((step - 1) / plan_steps) >= (int)plan.size());
    }
    void set_mastervol(uint8_t vol) {
        mastervol = vol;
    }
    int get_total_steps() {
        return plan_steps * plan.size();
    }
    void set_sustain_ms(int ms) {
        sustain_ms = ms;
    }
    Voice *get_voice(int num) {
        return &voices[CLIP(num, 0, 3)];
    }
};

class Sequencer {
private:
    int step;
    int initial_ms = 300;
    int final_ms = 300;
    int current_ms;
    int final_steps = 0;
    float shuffle = 0.5;
    bool playing = false;
    bool play_drums = true;
    absolute_time_t next_beat;
    absolute_time_t next_half;
public:
    Piece piece;

    inline Sequencer() {
        piece.make();
    };

    void mix(int32_t *out_buffer) {
        piece.mix(out_buffer);
        if(playing) {
            if(time_reached(next_beat)) {
                play_step();
                next_beat = make_timeout_time_ms(current_ms);
                next_half = make_timeout_time_ms(current_ms / 2 + current_ms * (shuffle / 5.0));
            }
            if(time_reached(next_half)) {
                play_step();
                next_half = at_the_end_of_time;
            }
        }
    }

    void play_step() {
        piece.play_step(step, current_ms, play_drums);
        step++;
        if(playing && piece.finished(step)) playing = false;
        int total_steps = piece.get_total_steps();
        int first_final = total_steps - final_steps;
        if(final_steps && step >= first_final) {
            current_ms = initial_ms + ((final_ms - initial_ms) * (step - first_final)) / final_steps;
        }
    }

    void set_shuffle(float s) {
        shuffle = s;
    }

    void set_play_drums(bool p) {
        play_drums = p;
    }

    void set_tempo_ms(int _ms, int _final_ms = 0, int _final_steps = 0) {
        initial_ms = _ms;
        final_ms = _final_ms ? _final_ms : initial_ms;
        final_steps = _final_steps;
    }

    void set_playing(bool p, bool once = false) {
        playing = p;
        if(p) step = 0;
        piece.set_once(once);
        next_half = at_the_end_of_time;
        next_beat = make_timeout_time_ms(0);
        current_ms = initial_ms;
    }

    void make_new_piece() {
        set_play_drums(true);
        piece.make();
        shuffle = (random() % 1000) / 1000.0;
        shuffle *= shuffle;
    }

    void config_reverb(uint16_t feedback, uint16_t volume) {
        piece.config_reverb(feedback, volume);
    }

    bool piece_finished() {
        return piece.finished(step);
    }

    void set_mastervol(uint8_t vol) {
        piece.set_mastervol(vol);
    }

    void play_happy() {
        piece.set_sustain_ms(800);
        set_tempo_ms(290, 400, 4);
        set_shuffle(0.6);
        set_play_drums(true);
        piece.make(0);
        Piece::plan_t plan{0, 0, 0, 3, 3, 3, 4, 4, 0, 0, 0, 0};
        piece.set_plan(2, plan);
        for(int i = 0; i < 4; i++) {
            piece.get_voice(i)->set_asr_ms(0, 400, 400);
            piece.get_voice(i)->set_wf_lfo_porta(2, 5, 0.03, 40);
            piece.get_voice(i)->set_filter(24, 0, 2, 50);
        }
        set_playing(true, true);
    }

    void play_sad() {
        piece.set_sustain_ms(800);
        set_tempo_ms(1500, 2000, 2);
        set_shuffle(0);
        set_play_drums(false);
        piece.make(2);
        Piece::plan_t plan{0, 4, 0, 0};
        piece.set_plan(1, plan);
        for(int i = 0; i < 4; i++) {
            piece.get_voice(i)->set_asr_ms(150, 800, 400);
            piece.get_voice(i)->set_wf_lfo_porta(1, 2.5, 0.15, 200);
            piece.get_voice(i)->set_filter(20, 0, 8, 800);
        }
        set_playing(true, true);
    }
};
