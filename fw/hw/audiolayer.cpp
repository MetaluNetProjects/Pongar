// audio layer

#include "sound.h"
#include "sound/audiolayer.h"
#include "sound/main_patch.h"
#include "pico/audio_pwm.h"
#include "pico/multicore.h"
#include "sound/osc.h"

struct audio_buffer_pool *producer_pool;

void sound_init(uint sample_freq, int buffer_size, int nb_buffers, int pin) {
    static audio_format_t audio_format = {
        .sample_freq = sample_freq,
        .format = AUDIO_BUFFER_FORMAT_PCM_S16,
        .channel_count = 1,
    };

    static struct audio_buffer_format producer_format = {
        .format = &audio_format,
        .sample_stride = 2
    };

    static audio_pwm_channel_config_t channel_config = default_mono_channel_config;
    channel_config.core.base_pin = pin;
    //channel_config.core.pio_sm = 1;

    producer_pool = audio_new_producer_pool(&producer_format, nb_buffers, buffer_size);
    bool __unused ok;

    const struct audio_format *output_format = audio_pwm_setup(&audio_format, -1, &channel_config);
    if (!output_format) {
        panic("PicoAudio: Unable to open audio device.\n");
    }
    ok = audio_pwm_default_connect(producer_pool, false);
    assert(ok);
    audio_pwm_set_enabled(true);
}

#define clip(s) ((s) > 32767 ? 32767 : (s) < -32767 ? -32767 : (s))

static AudioLayer* core1_layer;
static void core1_task() {
    while(1) core1_layer->audio_task();
}

MainPatch main_patch;

AudioLayer::AudioLayer(): patch(main_patch) {}

void AudioLayer::init(int audio_pin) {
    Osc::setup();
    Blosc::setup();
    sound_init(AUDIO_SAMPLE_RATE, AUDIO_SAMPLES_PER_BUFFER, 3, audio_pin);
    core1_layer = this;
    multicore_launch_core1(core1_task);
}

void AudioLayer::receivebytes(const char* data, uint8_t len) {
    uint8_t c = fraise_get_uint8();
    switch(c) {
    case 1:
        print_cpu();
        break;
    /*case 2: {
            main_patch.buzzer.osc1.setFreq(fraise_get_int16());
            main_patch.buzzer.osc1.setVol(fraise_get_int16());
            main_patch.buzzer.osc2.setFreq(fraise_get_int16());
            main_patch.buzzer.osc2.setVol(fraise_get_int16());
        }
        break;
    case 3: {
            char type = fraise_get_uint8();
            switch(type) {
            case 1:
                main_patch.buzz();
                break;
            case 2:
                main_patch.bounce();
                break;
            case 3:
                main_patch.bouncer.bounce(fraise_get_int16(), fraise_get_int16(), fraise_get_int16());
                break;
            case 4:
                main_patch.tut.tut(fraise_get_int16(), fraise_get_int16());
                break;
            case 5:
                main_patch.buzzer.config(fraise_get_int16(), fraise_get_int16(), fraise_get_int16(), fraise_get_int16());
                break;
                //case 6: main_patch.seq.make_melodies(); break;
            }
        }
        break;*/
    case 4: {
            patch.command((SoundCommand)fraise_get_uint8(), fraise_get_int16(), fraise_get_int16(), fraise_get_int16());
        }
        break;
    case 5: set_volume(fraise_get_uint8()); break;
    }
}

void AudioLayer::command(SoundCommand c, int p1, int p2, int p3) {
//    patch.command(c, p1, p2, p3);
    const uint64_t timeout_us = 30000;
    multicore_fifo_push_timeout_us(((int)c & 0x0fff) | 0x1000, timeout_us);
    multicore_fifo_push_timeout_us((p1 & 0x0fff), timeout_us);
    multicore_fifo_push_timeout_us((p2 & 0x0fff), timeout_us);
    multicore_fifo_push_timeout_us((p3 & 0x0fff), timeout_us);
}

void AudioLayer::audio_task() {
    static uint32_t buf[4];
    static int bufcount = 0;
    struct audio_buffer *buffer = take_audio_buffer(producer_pool, true);
    int16_t *samples = (int16_t *) buffer->buffer->bytes;
    int32_t int_samples[AUDIO_SAMPLES_PER_BUFFER] = {0};
    absolute_time_t start = get_absolute_time();
    int vol = (volume * volume) / 16;

    mix(int_samples, 0);
    for (uint i = 0; i < buffer->max_sample_count; i++) {
        samples[i] = (clip(int_samples[i]) * vol) / (256 * 16);
    }

    int us = absolute_time_diff_us(start, get_absolute_time());
    cpu_avg += (us - cpu_avg) / 16.0;

    buffer->sample_count = buffer->max_sample_count;
    give_audio_buffer(producer_pool, buffer);

    while(multicore_fifo_rvalid()) {
        uint32_t word;
        if(! multicore_fifo_pop_timeout_us(1, &word)) break;
        if((word & 0xf000) == 0x1000) {
            buf[0] = word & 0x0fff;
            bufcount = 1;
        } else if(bufcount < 4) {
            buf[bufcount++] = word & 0x0fff;
            if(bufcount == 4) {
                patch.command((SoundCommand)buf[0], buf[1], buf[2], buf[3]);
            }
        }
    }
}

void AudioLayer::print_cpu() {
    printf("cpu %.2f%% (%.2f / %d us)\n",
           cpu_avg * ((100.0 * 1.0e-6 * AUDIO_SAMPLE_RATE) / AUDIO_SAMPLES_PER_BUFFER),
           cpu_avg,
           (1000000 * AUDIO_SAMPLES_PER_BUFFER) / AUDIO_SAMPLE_RATE
          );
}

