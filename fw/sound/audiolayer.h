// hardware audio layer

#pragma once
#include "fraise.h"
#include "patch.h"
#include "sound_command.h"

class AudioLayer {
private:
    float cpu_avg = 0;
    Patch &patch;
    uint8_t volume = 255;
public:
    AudioLayer();
    void init(int audio_pin);

    void audio_task();
    void receivebytes(const char* data, uint8_t len);
    void command(SoundCommand c, int p1, int p2, int p3);
    void mix(int32_t *out_buffer, int32_t *in_buffer = 0) {
        patch.mix(out_buffer, in_buffer);
    }
    void print_cpu();
    void set_volume(uint8_t vol) { volume = vol; }
    Patch &get_patch() {
        return patch;
    }
};

