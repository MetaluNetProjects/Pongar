#pragma once

#include <math.h>
#include <string.h>
#include "config.h"
#include "sound_command.h"

#define CLIP(x, min, max) MAX(MIN((x), (max)), (min))

class Osc {
public:
	Osc(uint32_t _freq = 0, int _vol = 0, int16_t _release = 0, int16_t _lfoamp = 0, int16_t _lfofreq = 0):
		vol(_vol), release(_release), lfoamp(_lfoamp), lfofreq(_lfofreq)
	{
		setFreq(_freq);
	}

	/*int16_t getSample() {
		pos += step + lfoval;
		if (pos >= pos_max) pos -= pos_max;
		else if(pos < 0) pos += pos_max;
		return ((vol * sine_wave_table[(pos >> 16u)%sin_table_len]) >> 16u);
	}*/

	/*void dsp_sin(int16_t *buffer) {
		for (uint i = 0; i < AUDIO_SAMPLES_PER_BUFFER; i++) {
		    pos += step + lfoval;
		    if (pos >= pos_max) pos -= pos_max;
		    else if(pos < 0) pos += pos_max;
		    *buffer++ = ((vol * sine_wave_table[(pos >> 16u)%sin_table_len]) >> 16u);
		}
	}

	void dsp_saw(int16_t *buffer) {
		for (uint i = 0; i < AUDIO_SAMPLES_PER_BUFFER; i++) {
		    pos += step + lfoval;
		    if (pos >= pos_max) pos -= pos_max;
		    else if(pos < 0) pos += pos_max;
		    *buffer++ = (vol * ((int16_t)(pos >> 11u) - 32768)) >> 16u;
		}
	}*/

	void mix_sin(int32_t *buffer) {
		for (uint i = 0; i < AUDIO_SAMPLES_PER_BUFFER; i++) {
		    pos += step + lfoval;
		    if (pos >= pos_max) pos -= pos_max;
		    else if(pos < 0) pos += pos_max;
		    *buffer++ += ((vol * sine_wave_table[(pos >> 16u)%sin_table_len]) >> 16u);
		}
	}

	void mix_saw(int32_t *buffer) {
		for (uint i = 0; i < AUDIO_SAMPLES_PER_BUFFER; i++) {
		    pos += step + lfoval;
		    if (pos >= pos_max) pos -= pos_max;
		    else if(pos < 0) pos += pos_max;
		    *buffer++ += (vol * ((int16_t)(pos >> 11u) - 32768)) >> 16u;
		}
	}

	void mix_squ(int32_t *buffer, int thres) {
		for (uint i = 0; i < AUDIO_SAMPLES_PER_BUFFER; i++) {
		    pos += step + lfoval;
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
		lfoval = ((int32_t)lfoamp * sine_wave_table[(lfopos >> 16u)%sin_table_len]) >> 8u;
		lfoval = (lfoval * (step >> 12)) >> 8; // make lfo amp proportional of osc freq
		return (vol > 64);
	}

	void setStep(int32_t _step) {
		step = _step;
	}
	void setFreq(int f) { // 
	    setStep(((float)f * sin_table_len * 0x10000) / AUDIO_SAMPLE_RATE);
	}

	void setFreq8(int f8) { // 16.8 fixed point in Hz
	    setStep(((int64_t)f8 * (sin_table_len * 0x10000 / 256)) / AUDIO_SAMPLE_RATE);
	}

	void setMidi(int note) { // 16.8 fixed point in Hz
	    setFreq8(mtof8_table[note % 135]);
	}

	void setVol(int _vol) { vol = _vol; }
	void setLfo(int freq, int amp) { lfofreq = freq; lfoamp = amp; }
	static void setup() {
		for (int i = 0; i < sin_table_len; i++) {
			sine_wave_table[i] = 32767 * cosf((i + sin_table_len / 4)* 2 * (float) (M_PI / sin_table_len));
		}
		for(int i = 0; i < 135; i++) {
			mtof8_table[i] = 256 * 8.1758 * pow(2, i / 12.0);
		}
	}

//private:
	static const int sin_table_len = 2048;
	static int16_t sine_wave_table[sin_table_len];
	static uint32_t mtof8_table[135];
	static const int32_t pos_max = 0x10000 * sin_table_len;

	int32_t step = 0;
	int32_t pos = 0;
	int vol = 128;
	uint16_t release;
	uint16_t lfoamp;
	uint16_t lfofreq;
	uint32_t lfopos = 0;
	int32_t lfoval = 0;
};

