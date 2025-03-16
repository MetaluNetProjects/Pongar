// hardware audio layer

#pragma once
#include "fraise.h"
#include "sound/patch.h"

class AudioLayer {
private:
    float cpu_avg;
    Patch &main_patch;
public:
    AudioLayer(Patch &patch): main_patch(patch) {}
    void init(int audio_pin);

    void audio_task();
    void receivebytes(const char* data, uint8_t len);
    void print_cpu();
};
