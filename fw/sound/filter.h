#pragma once

#include <math.h>
#include <string.h>
#include "osc.h"
#include "config.h"

#define CLIP(x, min, max) MAX(MIN((x), (max)), (min))

// Pd bp~ converted to integer computation
class Bandpass {
private:
    int32_t last, prev;
    float coef1, coef2, gain;
public:
    Bandpass(float f, float q, float g) {
        setFQ(f, q, g);
    }
    void mix(int32_t *out_buffer, int32_t *in_buffer = 0) {
        int64_t c1 = coef1 * 4096;
        int64_t c2 = coef2 * 4096;
        int32_t g = gain * 512;
        if(!in_buffer) in_buffer = out_buffer;
        for (uint i = 0; i < AUDIO_SAMPLES_PER_BUFFER; i++) {
            int32_t output =  *in_buffer++ + (c1 * last + c2 * prev) / 4096;
            *out_buffer++ += (g * output) / 512;
            prev = last;
            last = output;
        }
    }

    void filter(int32_t *out_buffer, int32_t *in_buffer = 0) {
        int64_t c1 = coef1 * 4096;
        int64_t c2 = coef2 * 4096;
        int32_t g = gain * 512;
        if(!in_buffer) in_buffer = out_buffer;
        for (uint i = 0; i < AUDIO_SAMPLES_PER_BUFFER; i++) {
            int32_t output =  *in_buffer++ + (c1 * last + c2 * prev) / 4096;
            *out_buffer++ = (g * output) / 512;
            prev = last;
            last = output;
        }
    }

    static float sigbp_qcos(float f)
    {
        if (f >= -(0.5f * 3.14159f) && f <= 0.5f * 3.14159f)
        {
            float g = f * f;
            return (((g * g * g * (-1.0f / 720.0f) + g * g * (1.0f / 24.0f)) - g * 0.5) + 1);
        }
        else return (0);
    }

    void setFQ(float f, float q, float g) {
        float r, oneminusr, omega;
        if (f < 0.001) f = 10;
        if (q < 0) q = 0;
        omega = f * (2.0f * 3.14159f) / AUDIO_SAMPLE_RATE;
        if (q < 0.001) oneminusr = 1.0f;
        else oneminusr = omega / q;
        if (oneminusr > 1.0f) oneminusr = 1.0f;
        r = 1.0f - oneminusr;
        coef1 = 2.0f * sigbp_qcos(omega) * r;
        coef2 = - r * r;
        gain = 2 * oneminusr * (oneminusr + r * omega) /*!!!*/ * g /*!!!*/;
    }

    void setMidiQ(int note, float q, float g) {
        setFQ(Osc::mtof8(note) / 256, q, g);
    }
};

// Pd hip~ converted to integer computation
class Hip {
private:
    int32_t last;
    uint16_t coeff;
public:
    Hip(int f) {
        setFreq(f);
    }
    void mix(int32_t *out_buffer, int32_t *in_buffer = 0) {
        if(!in_buffer) in_buffer = out_buffer;
        for (uint i = 0; i < AUDIO_SAMPLES_PER_BUFFER; i++) {
            int32_t new_sample = *in_buffer++ + (coeff * last) / 256;
            *out_buffer++ += new_sample - last;
            last = new_sample;
        }
    }
    void filter(int32_t *out_buffer, int32_t *in_buffer = 0) {
        if(!in_buffer) in_buffer = out_buffer;
        for (uint i = 0; i < AUDIO_SAMPLES_PER_BUFFER; i++) {
            int32_t new_sample = *in_buffer++ + (coeff * last) / 256;
            *out_buffer++ = new_sample - last;
            last = new_sample;
        }
    }
    void setFreq(int f) {
        coeff = 256 * (1.0 - f * (2 * 3.14159) / AUDIO_SAMPLE_RATE);
        coeff = CLIP(coeff, 0, 255);
    }
    void setMidi(int note) {
        setFreq(Osc::mtof8(note) / 256);
    }
};

// Pd lop~ converted to integer computation
class Lop {
private:
    int32_t last;
    uint16_t coeff;
public:
    Lop(int f) {
        setFreq(f);
    }
    void mix(int32_t *out_buffer, int32_t *in_buffer = 0) {
        if(!in_buffer) in_buffer = out_buffer;
        int feedback = 255 - coeff;
        for (uint i = 0; i < AUDIO_SAMPLES_PER_BUFFER; i++) {
            last = *out_buffer++ = (coeff * *in_buffer++ + feedback * last) / 256;
        }
    }
    void filter(int32_t *out_buffer, int32_t *in_buffer = 0) {
        if(!in_buffer) in_buffer = out_buffer;
        int feedback = 255 - coeff;
        for (uint i = 0; i < AUDIO_SAMPLES_PER_BUFFER; i++) {
            last = *out_buffer++ = (coeff * *in_buffer++ + feedback * last) / 256;
        }
    }
    void setFreq(int f) {
        coeff = (256 * f * 2 * 3.14159) / AUDIO_SAMPLE_RATE;
        coeff = CLIP(coeff, 0, 255);
    }

    void setMidi(int note) {
        setFreq(Osc::mtof8(note) / 256);
    }
};

