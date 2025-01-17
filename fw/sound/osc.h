#pragma once

#include <math.h>
#include "config.h"

class Osc {
public:
	Osc(uint32_t _freq = 0, int _vol = 0, int16_t _release = 0, int16_t _lfoamp = 0, int16_t _lfofreq = 0):
		vol(_vol), release(_release), lfoamp(_lfoamp), lfofreq(_lfofreq)
	{
		setFreq(_freq);
	}
	int16_t getSample();
	void dsp_sin(int16_t *buffer);
	void dsp_saw(int16_t *buffer);
	void mix_sin(int32_t *buffer);
	void mix_saw(int32_t *buffer);
	bool update();
	void setStep(int32_t _step) {
		step = _step;
	}
	void setFreq(int f) { // 
	    setStep(((float)f * 2048 * 0x10000) / AUDIO_SAMPLE_RATE);
	}

	void setFreq8(int f8) { // 16.8 fixed point in Hz
	    setStep(((int64_t)f8 * (2048 * 0x10000 / 256)) / AUDIO_SAMPLE_RATE);
	}

	void setVol(int _vol) { vol = _vol; }

	static void setup() {
		//printf("Osc setup sine table\n");
		for (int i = 0; i < sin_table_len; i++) {
			sine_wave_table[i] = 32767 * cosf((i + sin_table_len / 4)* 2 * (float) (M_PI / sin_table_len));
		}
	}

//private:
	static const int sin_table_len = 2048;
	static int16_t sine_wave_table[sin_table_len];
	static const int32_t pos_max = 0x10000 * sin_table_len;

	int32_t step = 0;
	int32_t pos = 0;
	int vol = 128;
	uint16_t release;
	uint16_t lfoamp;
	uint16_t lfofreq;
	uint32_t lfopos;
	int32_t lfoval;
};

class Patch {
  public:
    virtual void mix(int32_t *out_buffer, int32_t *in_buffer = 0) = 0;// {};
};

class Buzzer : public Patch {
  private:
  public:
    Osc osc1;
    Osc osc2;
    absolute_time_t stop_time;
    Buzzer() : osc1(100, 20000), osc2(103, -20000) { /*osc1.setVol(20000); osc1.setVol(-20000);*/}
    virtual void mix(int32_t *out_buffer, int32_t *in_buffer = 0) {
        if(time_reached(stop_time)) return;
        osc1.mix_saw(out_buffer);
        osc2.mix_saw(out_buffer);
    }
    void buzz(uint ms) {
        stop_time = make_timeout_time_ms(ms);
    }
};

class EnveloppeOld : public Patch {
  public:
    int Sms;
    float Ainc, Sinc, Rinc;
    float level;
    enum {OFF, A, S, R} state = OFF;
    absolute_time_t next_time;
    virtual void mix(int32_t *out_buffer, int32_t *in_buffer) {
        for (uint i = 0; i < AUDIO_SAMPLES_PER_BUFFER; i++) {
            switch(state) {
                case OFF: level = 0; break;
                case A:
                    level += Ainc;
                    if(level >= 1.0) {
                        level = 1;
                        next_time = make_timeout_time_ms(Sms);
                        state = S;
                    }
                    break;
                case S:
                    level = 1.0;
                    if(time_reached(next_time)) state = R;
                    break;
                case R:
                    level -= Rinc;
                    if(level <= 0.0) {
                        level = 0;
                        state = OFF;
                    }
            }
            *out_buffer++ += *in_buffer++ * level;
        }
    }

    void start(int a, int s, int r) {
        if(a < 1) a = 1;
        Ainc = 1000.0 / (a * AUDIO_SAMPLE_RATE);
        Sms = s;
        if(r < 1) r = 1;
        Rinc = 1000.0 / (r * AUDIO_SAMPLE_RATE);
        level = 0;
        state = A;
    }
};

class Enveloppe : public Patch {
  public:
    int Sms;
    int Ainc, Sinc, Rinc;
    int level;
    static const int MAX_LEVEL_BITS = 24;
    static const int MAX_LEVEL = 1 << MAX_LEVEL_BITS;
   enum {OFF, A, S, R} state = OFF;
    absolute_time_t next_time;
    virtual void mix(int32_t *out_buffer, int32_t *in_buffer) {
        for (uint i = 0; i < AUDIO_SAMPLES_PER_BUFFER; i++) {
            switch(state) {
                case OFF: level = 0; break;
                case A:
                    level += Ainc;
                    if(level >= MAX_LEVEL) {
                        level = MAX_LEVEL;
                        next_time = make_timeout_time_ms(Sms);
                        state = S;
                    }
                    break;
                case S:
                    level = MAX_LEVEL;
                    if(time_reached(next_time)) state = R;
                    break;
                case R:
                    level -= Rinc;
                    if(level <= 0) {
                        level = 0;
                        state = OFF;
                    }
            }
            *out_buffer++ += (*in_buffer++ * (level >> (MAX_LEVEL_BITS - 10))) / (1 << 10);
        }
    }

    void start(int a, int s, int r) {
        if(a < 1) a = 1;
        Ainc = MAX_LEVEL / (a * (AUDIO_SAMPLE_RATE / 1000));
        Sms = s;
        if(r < 1) r = 1;
        Rinc = MAX_LEVEL / (r * (AUDIO_SAMPLE_RATE / 1000));
        level = 0;
        state = A;
    }
};

class Bouncer : public Patch {
  private:
    Osc osc1;
    Enveloppe env;
    int freq;
    int freq_inc;
  public:
    //absolute_time_t stop_time;
    Bouncer() : osc1(500, 20000) {}
    virtual void mix(int32_t *out_buffer, int32_t *in_buffer = 0) {
        //if(time_reached(stop_time)) return;
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
    }
};

class MainPatch : public Patch {
  private:
  public:
    Buzzer buzzer;
    Bouncer bouncer;
    virtual void mix(int32_t *out_buffer, int32_t *in_buffer = 0) {
        buzzer.mix(out_buffer);
        bouncer.mix(out_buffer);
    }
    void buzz() {
        buzzer.buzz(400);
    }
    void bounce(bool way = true) {
        if(way) bouncer.bounce(120, 200, 5000);
        else bouncer.bounce(120, 1000, -5000);
    }
};
