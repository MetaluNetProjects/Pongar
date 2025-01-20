// audio layer

#include "sound.h"
#include "pico/audio_pwm.h"
#include "pico/multicore.h"

#include "osc.h"
#include "../config.h"

//#define SAMPLES_PER_BUFFER 256

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

void AudioLayer::init(int audio_pin, int tx_pin) {
    sound_init(AUDIO_SAMPLE_RATE, AUDIO_SAMPLES_PER_BUFFER, 3, audio_pin);
    player.init(tx_pin);
    core1_layer = this;
    multicore_launch_core1(core1_task);
}

void AudioLayer::receivebytes(const char* data, uint8_t len) {
    char c = data[0];
    //printf("audio rcv %d\n", c);
    if(c > 9) {
        player.receivebytes(data, len);
        return;
    }

    c = fraise_get_uint8();
    switch(c) {
        case 1: print_cpu(); break;
        case 2: {
                main_patch.buzzer.osc1.setFreq(fraise_get_int16());
                main_patch.buzzer.osc1.setVol(fraise_get_int16());
                main_patch.buzzer.osc2.setFreq(fraise_get_int16());
                main_patch.buzzer.osc2.setVol(fraise_get_int16());
            } break;
        case 3: {
            char type = fraise_get_uint8();
            switch(type) {
                case 1: main_patch.buzz(); break;
                case 2: main_patch.bounce(); break;
                case 3: main_patch.bouncer.bounce(fraise_get_int16(), fraise_get_int16(), fraise_get_int16()); break;
            }
        }
    }
}

//extern Osc osc1;

void AudioLayer::audio_task() {
    struct audio_buffer *buffer = take_audio_buffer(producer_pool, true);
    int16_t *samples = (int16_t *) buffer->buffer->bytes;
    int32_t int_samples[AUDIO_SAMPLES_PER_BUFFER] = {0};
    absolute_time_t start = get_absolute_time();
    //int32_t s;
    //for (uint i = 0; i < buffer->max_sample_count; i++) {
        /*s = ((int32_t)vol * voice_getSample()) >> 8u; s = clip(s);
        s = s + echo1.getSample(s); s = clip(s);
        s = s + echo2.getSample(s); s = clip(s);
        samples[i] = s;*/
        //s = osc1.getSample();
        /*samples[i] = osc1.getSample(); //clip(s);
    }*/
    //osc1.mix_sin(int_samples);
    main_patch.mix(int_samples, 0);
    for (uint i = 0; i < buffer->max_sample_count; i++) {
        samples[i] = clip(int_samples[i]);
    }

    int us = absolute_time_diff_us(start, get_absolute_time());
    cpu_avg += (us - cpu_avg) / 16.0;

    buffer->sample_count = buffer->max_sample_count;
    give_audio_buffer(producer_pool, buffer);
}

void AudioLayer::update() {
    player.update();
}

void AudioLayer::print_cpu() {
    printf("cpu %.2f%% (%.2f / %d us)\n",
        cpu_avg * ((100.0 * 1.0e-6 * AUDIO_SAMPLE_RATE) / AUDIO_SAMPLES_PER_BUFFER),
        cpu_avg,
        (1000000 * AUDIO_SAMPLES_PER_BUFFER) / AUDIO_SAMPLE_RATE
    );
}

bool AudioLayer::player_is_playing() {
    return player.is_playing();
}

void AudioLayer::command(SoundCommand c, int p1, int p2, int p3) {
    switch(c) {
        case SoundCommand::say:
            player.play(1, p1);
            break;
        case SoundCommand::saypause:
            player.silence(p1);
            break;
        case SoundCommand::sayclear:
            player.clear();
            break;
        case SoundCommand::buzz:
            main_patch.buzz();
            break;
        case SoundCommand::bounce:
            main_patch.bounce(p1 > 0);
            break;
        default: ;
    }
}
