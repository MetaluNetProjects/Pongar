//#include "pico/stdlib.h"
#include "stdlib.h"
#include <stdio.h>
#include <math.h>
#include <list>
#include "fraise.h"
#include "osc.h"

int16_t Osc::sine_wave_table[Osc::sin_table_len];
std::list<Osc> oscList;

int16_t Osc::getSample() {
	pos += step + lfoval;
	if (pos >= pos_max) pos -= pos_max;
	else if(pos < 0) pos += pos_max;
	return ((vol * sine_wave_table[(pos >> 16u)%sin_table_len]) >> 16u);
}

void Osc::dsp_sin(int16_t *buffer) {
    for (uint i = 0; i < AUDIO_SAMPLES_PER_BUFFER; i++) {
        pos += step + lfoval;
        if (pos >= pos_max) pos -= pos_max;
        else if(pos < 0) pos += pos_max;
        *buffer++ = ((vol * sine_wave_table[(pos >> 16u)%sin_table_len]) >> 16u);
    }
}

void Osc::dsp_saw(int16_t *buffer) {
    for (uint i = 0; i < AUDIO_SAMPLES_PER_BUFFER; i++) {
        pos += step + lfoval;
        if (pos >= pos_max) pos -= pos_max;
        else if(pos < 0) pos += pos_max;
        *buffer++ = (vol * ((int16_t)(pos >> 11u) - 32768)) >> 16u;
    }
}

void Osc::mix_sin(int32_t *buffer) {
    for (uint i = 0; i < AUDIO_SAMPLES_PER_BUFFER; i++) {
        pos += step + lfoval;
        if (pos >= pos_max) pos -= pos_max;
        else if(pos < 0) pos += pos_max;
        *buffer++ += ((vol * sine_wave_table[(pos >> 16u)%sin_table_len]) >> 16u);
    }
}

void Osc::mix_saw(int32_t *buffer) {
    for (uint i = 0; i < AUDIO_SAMPLES_PER_BUFFER; i++) {
        pos += step + lfoval;
        if (pos >= pos_max) pos -= pos_max;
        else if(pos < 0) pos += pos_max;
        *buffer++ += (vol * ((int16_t)(pos >> 11u) - 32768)) >> 16u;
    }
}

bool Osc::update() {
	if(release && vol > 64) {
		vol = (vol * release) >> 16;
	}
	lfopos += lfofreq << 12;
	if (lfopos >= pos_max) lfopos -= pos_max;
	lfoval = ((int32_t)lfoamp * sine_wave_table[(lfopos >> 16u)%sin_table_len]) >> 8u;
	return (vol > 64);
}


