// hardware audio layer

#pragma once
#include "fraise.h"
#include "sound/main_patch.h"
#include "sound/sound_command.h"

class AudioLayer {
private:
    float cpu_avg;
public:
    MainPatch main_patch;
    void init(int audio_pin);

    void audio_task();
    void receivebytes(const char* data, uint8_t len);
    void print_cpu();
    void command(SoundCommand c, int p1 = 0, int p2 = 0, int p3 = 0);
};
