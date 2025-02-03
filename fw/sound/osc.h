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
	int16_t getSample();
	void dsp_sin(int16_t *buffer);
	void dsp_saw(int16_t *buffer);
	void mix_sin(int32_t *buffer);
	void mix_saw(int32_t *buffer);
	void mix_squ(int32_t *buffer, int thres);
	bool update();
	void setStep(int32_t _step) {
		step = _step;
	}
	void setFreq(int f) { // 
	    setStep(((float)f * sin_table_len * 0x10000) / AUDIO_SAMPLE_RATE);
	}

	void setFreq8(int f8) { // 16.8 fixed point in Hz
	    setStep(((int64_t)f8 * (sin_table_len * 0x10000 / 256)) / AUDIO_SAMPLE_RATE);
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
	uint32_t lfopos = 0;
	int32_t lfoval = 0;
};

class Patch {
  public:
    virtual void mix(int32_t *out_buffer, int32_t *in_buffer = 0) = 0;// {};
};

  // Pd bp~ converted to integer computation
class Bandpass : public Patch {
  private:
    int32_t last, prev;
    float coef1, coef2, gain;
  public:
    Bandpass(float f, float q) { setFQ(f, q);}
    virtual void mix(int32_t *out_buffer, int32_t *in_buffer = 0) {
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
        if (f >= -(0.5f*3.14159f) && f <= 0.5f*3.14159f)
        {
            float g = f*f;
            return (((g*g*g * (-1.0f/720.0f) + g*g*(1.0f/24.0f)) - g*0.5) + 1);
        }
        else return (0);
    }

    void setFQ(float f, float q) {
        float r, oneminusr, omega;
        if (f < 0.001) f = 10;
        if (q < 0) q = 0;
        omega = f * (2.0f * 3.14159f) / AUDIO_SAMPLE_RATE;
        if (q < 0.001) oneminusr = 1.0f;
        else oneminusr = omega/q;
        if (oneminusr > 1.0f) oneminusr = 1.0f;
        r = 1.0f - oneminusr;
        coef1 = 2.0f * sigbp_qcos(omega) * r;
        coef2 = - r * r;
        gain = 2 * oneminusr * (oneminusr + r * omega) /*!!!*/ * q /*!!!*/;
    }
};

class Hip : public Patch {
  private:
    int32_t last;
    uint16_t coeff;
  public:
    Hip(int f) { setFreq(f);}
    virtual void mix(int32_t *out_buffer, int32_t *in_buffer = 0) {
        for (uint i = 0; i < AUDIO_SAMPLES_PER_BUFFER; i++) {
            int32_t new_sample = *out_buffer + (coeff * last) / 256;
            *out_buffer++ = new_sample - last;
            last = new_sample;
        }
    }
    void setFreq(int f) {
        coeff = 256 * (1.0 - f * (2 * 3.14159) / AUDIO_SAMPLE_RATE);
        coeff = CLIP(coeff, 0, 255);
    }
};

class Buzzer : public Patch {
  private:
  public:
    Osc osc1;
    Osc osc2;
    Hip hip1;
    Bandpass bp1;
    absolute_time_t stop_time;
    int32_t buf[AUDIO_SAMPLES_PER_BUFFER];
    int32_t buf2[AUDIO_SAMPLES_PER_BUFFER];
    int gain = 3 * 256;
    int squthres = 0;
    Buzzer() : osc1(100, 20000), osc2(103, -20000), hip1(600), bp1(1111, 500) {}
    virtual void mix(int32_t *out_buffer, int32_t *in_buffer = 0) {
        if(time_reached(stop_time)) return;
        memset(buf, 0, sizeof(buf));
        //osc1.mix_squ(buf, squthres);
        osc1.mix_saw(buf);
        osc2.mix_saw(buf);
        hip1.mix(buf);
        bp1.mix(buf2, buf);
        for (uint i = 0; i < AUDIO_SAMPLES_PER_BUFFER; i++) {
            *out_buffer++ = CLIP(((buf[i] + buf2[i] / 16) * gain) / 256, -65536, 65535);
        }
        osc1.setFreq(85 + random()%5);
        osc2.setFreq(91 + random()%5);
    }
    void buzz(uint ms) {
        stop_time = make_timeout_time_ms(ms);
    }
    
    void config(int f, int thr, int hipf, int g) {
        osc1.setFreq(f);
        //osc2.setFreq(f2);
        squthres = thr;
        hip1.setFreq(hipf);
        gain = g;
    }
};

class Ring: public Patch {
  public:
    Osc osc1;
    Bandpass bp1;
    int32_t buf[AUDIO_SAMPLES_PER_BUFFER];
    Ring(): osc1(20, 20000), bp1(1000, 100) {}
    virtual void mix(int32_t *out_buffer, int32_t *in_buffer = 0) {
        //if(time_reached(stop_time)) return;
        memset(buf, 0, sizeof(buf));
        osc1.mix_saw(buf);
        bp1.mix(buf);
        for (uint i = 0; i < AUDIO_SAMPLES_PER_BUFFER; i++) {
            *out_buffer++ += CLIP(buf[i], -65536, 65535);
        }
    }
};

class Tut : public Patch {
  private:
  public:
    Osc osc1;
    absolute_time_t stop_time;
    Tut() : osc1(1000, 20000) {}
    virtual void mix(int32_t *out_buffer, int32_t *in_buffer = 0) {
        if(time_reached(stop_time)) return;
        osc1.mix_saw(out_buffer);
    }
    void tut(uint f, uint ms) {
        osc1.setFreq(f);
        stop_time = make_timeout_time_ms(ms);
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
    Tut tut;
    Ring ring;
    virtual void mix(int32_t *out_buffer, int32_t *in_buffer = 0) {
        buzzer.mix(out_buffer);
        bouncer.mix(out_buffer);
        tut.mix(out_buffer);
        //ring.mix(out_buffer);
    }
    void buzz() {
        buzzer.buzz(400);
    }
    void bounce(bool way = true) {
        if(way) bouncer.bounce(120, 200, 5000);
        else bouncer.bounce(120, 1000, -3000);
    }

    void command(SoundCommand c, int p1, int p2, int p3) {
        //printf("mainpatch cmd %d %d %d %d\n", (int)c, p1, p2, p3);
        switch(c) {
            case SoundCommand::buzz: buzz(); break;
            case SoundCommand::bounce: bounce(p1 > 0); break;
            case SoundCommand::tut: tut.tut(p1, p2); break;
            default: ;
        }
    }
};
