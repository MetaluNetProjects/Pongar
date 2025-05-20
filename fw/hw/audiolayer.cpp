// audio layer

#include "sound.h"
#include "sound/audiolayer.h"
#include "sound/main_patch.h"
#include "pico/audio_pwm.h"
#include "pico/multicore.h"
#include "pico/util/queue.h"

#define printf fraise_printf

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

bool AudioLayer::talkover = false;

static AudioLayer* core1_layer;

static void core1_task() {
    multicore_lockout_victim_init(); // allow core0 to lockout core1, for flash programming purpose.
    while(1) core1_layer->audio_task();
}

MainPatch main_patch;

AudioLayer::AudioLayer(): patch(main_patch) {}

static queue_t to_audio_queue;

struct queue_event_t {
    SoundCommand c;
    int p1, p2, p3;
};

void AudioLayer::init(int audio_pin) {
    sound_init(AUDIO_SAMPLE_RATE, AUDIO_SAMPLES_PER_BUFFER, 3, audio_pin);
    core1_layer = this;
    queue_init(&to_audio_queue, sizeof(queue_event_t), 64);
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
    case 6: printf("audio volume %d %d %d\n", volume, talkover, talkover_vol); break;
    }
}

void AudioLayer::command(SoundCommand c, int p1, int p2, int p3) {
    queue_event_t event{c, p1, p2, p3};
    queue_add_blocking(&to_audio_queue, &event);
}

void AudioLayer::audio_task() {
    struct audio_buffer *buffer = take_audio_buffer(producer_pool, true);
    int16_t *samples = (int16_t *) buffer->buffer->bytes;
    int32_t int_samples[AUDIO_SAMPLES_PER_BUFFER] = {0};
    absolute_time_t start = get_absolute_time();

    mix(int_samples, 0);

    if(talkover) {
        static const int DECREASE_RATE = 5;
        if(talkover_vol > (TALKOVER_MIN_VOL + DECREASE_RATE)) talkover_vol -= DECREASE_RATE;
        else talkover_vol = TALKOVER_MIN_VOL;
    } else {
        static const int INCREASE_RATE = 1;
        if(talkover_vol < (255 - INCREASE_RATE)) talkover_vol += INCREASE_RATE;
        else talkover_vol = 255;
    }

    int vol = (volume * (int)volume * (int)talkover_vol) / (16 * 255);

    for (uint i = 0; i < buffer->max_sample_count; i++) {
        samples[i] = (clip(int_samples[i]) * vol) / (256 * 16);
    }

    int us = absolute_time_diff_us(start, get_absolute_time());
    cpu_avg += (us - cpu_avg) / 16.0;

    buffer->sample_count = buffer->max_sample_count;
    give_audio_buffer(producer_pool, buffer);

    queue_event_t event;
    if(queue_try_remove(&to_audio_queue, &event)) {
        patch.command(event.c, event.p1, event.p2, event.p3);
    }
}

void AudioLayer::print_cpu() {
    printf("cpu %.2f%% (%.2f / %d us)\n",
           cpu_avg * ((100.0 * 1.0e-6 * AUDIO_SAMPLE_RATE) / AUDIO_SAMPLES_PER_BUFFER),
           cpu_avg,
           (1000000 * AUDIO_SAMPLES_PER_BUFFER) / AUDIO_SAMPLE_RATE
          );
}

