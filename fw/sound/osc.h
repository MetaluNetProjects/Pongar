#pragma once

#include "sound.h"
#include <cmath>
#include <string>
#include <array>

#define CLIP(x, min, max) MAX(MIN((x), (max)), (min))

template<int SIZE, typename T, typename Lambda> std::array<T, SIZE> build_lut(Lambda lambda) {
    std::array<T, SIZE> array{};
    for (int i = 0; i < SIZE; i++) array[i] = lambda(i);
    return array;
}

class Osc {
private:
    static const int mtof_table_div = 8;
    static const int mtof_table_len = 135 * mtof_table_div;
    inline static auto mtof8_table = build_lut<mtof_table_len, uint32_t>(
        [](int i) {
            return 256 * 8.1758 * pow(2, i / 12.0 / (float)mtof_table_div);
        }
    );

    uint16_t release;
    uint16_t lfoamp;
    uint16_t lfofreq;
    uint32_t lfopos = 0;
protected:
    static const int sin_table_len = 2048;
    inline static auto sin_table = build_lut<sin_table_len, int16_t>(
        [](int i) {
            return 32767 * sinf(i * 2.0f * M_PI / sin_table_len);
        }
    );
    static const int32_t pos_max = 0x10000 * sin_table_len;
    int32_t increment = 0;
    int32_t pos = 0;
    int vol = 128;
    int32_t lfoval = 0;

public:
    Osc(uint32_t _freq = 0, int _vol = 0, int16_t _release = 0, int16_t _lfoamp = 0, int16_t _lfofreq = 0):
        release(_release), lfoamp(_lfoamp), lfofreq(_lfofreq), vol(_vol)
    {
        setFreq(_freq);
    }

    static int mtof8(float note) {
        return mtof8_table[(int)(note * mtof_table_div) % mtof_table_len];
    }

    void mix_sin(int32_t *buffer) {
        for (uint i = 0; i < AUDIO_SAMPLES_PER_BUFFER; i++) {
            pos += increment + lfoval;
            if (pos >= pos_max) pos -= pos_max;
            else if(pos < 0) pos += pos_max;
            *buffer++ += ((vol * sin_table[(pos >> 16u) % sin_table_len]) >> 15u);
        }
    }

    void mix_saw(int32_t *buffer) {
        for (uint i = 0; i < AUDIO_SAMPLES_PER_BUFFER; i++) {
            pos += increment + lfoval;
            if (pos >= pos_max) pos -= pos_max;
            else if(pos < 0) pos += pos_max;
            *buffer++ += (vol * ((int16_t)(pos >> 11u))) >> 15u;
        }
    }

    void mix_squ(int32_t *buffer, int thres) {
        for (uint i = 0; i < AUDIO_SAMPLES_PER_BUFFER; i++) {
            pos += increment + lfoval;
            if (pos >= pos_max) pos -= pos_max;
            else if(pos < 0) pos += pos_max;
            if((pos >> 11u) < thres) *buffer++ -= vol;
            else *buffer++ += vol;
        }
    }

    bool update() {
        if(release && vol > 64) {
            vol = (vol * release) >> 16;
        }
        lfopos += lfofreq << 12;
        if (lfopos >= pos_max) lfopos -= pos_max;
        lfoval = ((int32_t)lfoamp * sin_table[(lfopos >> 16u) % sin_table_len]) >> 8u;
        lfoval = (lfoval * (increment >> 12)) >> 8; // make lfo amp proportional of osc freq
        return (vol > 64);
    }

    void setIncrement(int32_t incr) {
        increment = incr;
    }
    void setFreq(int f) { //
        setIncrement(((float)f * sin_table_len * 0x10000) / AUDIO_SAMPLE_RATE);
    }

    void setFreq8(int f8) { // 16.8 fixed point in Hz
        setIncrement(((int64_t)f8 * (sin_table_len * 0x10000 / 256)) / AUDIO_SAMPLE_RATE);
    }

    void setMidi(int note) { // 16.8 fixed point in Hz
        setFreq8(mtof8(note));
    }

    void setVol(int _vol) {
        vol = _vol;
    }

    void setLfo(float freq, float amp) {
        lfofreq = freq * (sin_table_len * 16 * AUDIO_SAMPLES_PER_BUFFER) / AUDIO_SAMPLE_RATE;
        lfoamp = CLIP(amp, 0, 1) * 1000;
    }
};

class Blosc : public Osc {
private:
    static const int transition_table_len = 1024;
    inline static auto transition_table = build_lut<transition_table_len, int16_t>(
        [](int i) {
            float x = M_PI * i / (float)transition_table_len;
            return 32767 * (0.75 * (0.3333 * cos(3 * x) - cosf(x)));
        }
    );
    float bandlimit = 1.0;

public:
    void mix_blsaw(int32_t *buffer) {
        int mult = (0.2 * bandlimit * sin_table_len * 0x10000 * 128) / (1 + increment);
        if(mult < 1) mult = 1;
        /*
            mult = (AUDIO_SAMPLE_RATE / 2 * 0.4 * bandlimit) / f;
            increment = (f * sin_table_len * 0x10000) / AUDIO_SAMPLE_RATE);
        */
        for (uint i = 0; i < AUDIO_SAMPLES_PER_BUFFER; i++) {
            pos += increment + lfoval;
            if (pos >= pos_max) pos -= pos_max;
            else if(pos < 0) pos += pos_max;
            int phase = (int16_t)(pos >> 11u); // -32768 32767
            int trans_index = (phase * mult) >> 7u;
            trans_index = ((CLIP(trans_index, -32768, 32767) + 32768) * transition_table_len) >> 16u;
            trans_index = CLIP(trans_index, 0, transition_table_len - 1);
            int16_t trans = transition_table[trans_index];
            *buffer++ += ((vol * (phase - 2 * trans)) >> 15u);
        }
    }

    void mix_blsqu(int32_t *buffer, int thres) {
        int mult = (0.1 * bandlimit * sin_table_len * 0x10000 * 128) / (1 + increment);
        if(mult < 1) mult = 1;
        for (uint i = 0; i < AUDIO_SAMPLES_PER_BUFFER; i++) {
            pos += increment + lfoval;
            if (pos >= pos_max) pos -= pos_max;
            else if(pos < 0) pos += pos_max;
            int phase = sin_table[(pos >> 16u) % sin_table_len] + thres; // -32768 32767
            int trans_index = ((phase * mult) >> 7u);
            trans_index = ((CLIP(trans_index, -32768, 32767) + 32768) * transition_table_len) >> 16u;
            trans_index = CLIP(trans_index, 0, transition_table_len - 1);
            int16_t trans = transition_table[trans_index];
            *buffer++ += ((vol * 2 * trans) >> 15u);
        }
    }

    void set_bandlimit(float b) {
        bandlimit = b;
    }
};

